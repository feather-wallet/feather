// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WindowManager.h"

#include <QMessageBox>

#include "constants.h"
#include "dialog/PasswordDialog.h"
#include "dialog/SplashDialog.h"
#include "utils/Icons.h"
#include "utils/NetworkManager.h"
#include "utils/os/tails.h"
#include "utils/TorManager.h"
#include "utils/WebsocketNotifier.h"

WindowManager::WindowManager() {
    m_walletManager = WalletManager::instance();
    m_splashDialog = new SplashDialog;

    connect(m_walletManager, &WalletManager::walletOpened,        this, &WindowManager::onWalletOpened);
    connect(m_walletManager, &WalletManager::walletCreated,       this, &WindowManager::onWalletCreated);
    connect(m_walletManager, &WalletManager::deviceButtonRequest, this, &WindowManager::onDeviceButtonRequest);
    connect(m_walletManager, &WalletManager::deviceError,         this, &WindowManager::onDeviceError);

    connect(qApp, &QGuiApplication::lastWindowClosed, this, &WindowManager::quitAfterLastWindow);

    m_tray = new QSystemTrayIcon(icons()->icon("appicons/64x64.png"));
    m_tray->setToolTip("Feather Wallet");
    this->buildTrayMenu();
    m_tray->show();

    this->initSkins();

    if (!config()->get(Config::firstRun).toBool() || TailsOS::detect() || WhonixOS::detect()) {
        this->onInitialNetworkConfigured();
    }

    this->startupWarning();

    if (!this->autoOpenWallet()) {
        this->initWizard();
    }
}

// ######################## APPLICATION LIFECYCLE ########################

void WindowManager::quitAfterLastWindow() {
    if (m_windows.length() > 0 || m_openingWallet) {
        return;
    }

    qDebug() << "No wizards in progress and no wallets open, quitting application.";
    this->close();
}

void WindowManager::close() {
    qDebug() << Q_FUNC_INFO;
    for (const auto &window: m_windows) {
        window->close();
    }

    torManager()->stop();
    m_tray->hide();

    QApplication::quit();
}

void WindowManager::closeWindow(MainWindow *window) {
    m_windows.removeOne(window);
}

void WindowManager::restartApplication(const QString &binaryFilename) {
    QProcess::startDetached(binaryFilename, qApp->arguments());
    this->close();
}

void WindowManager::startupWarning() {
    // Stagenet / Testnet
    auto worthlessWarning = QString("Feather wallet is currently running in %1 mode. This is meant "
                                    "for developers only. Your coins are WORTHLESS.");
    if (constants::networkType == NetworkType::STAGENET && config()->get(Config::warnOnStagenet).toBool()) {
        this->showWarningMessageBox("Warning", worthlessWarning.arg("stagenet"));
        config()->set(Config::warnOnStagenet, false);
    }
    else if (constants::networkType == NetworkType::TESTNET && config()->get(Config::warnOnTestnet).toBool()){
        this->showWarningMessageBox("Warning", worthlessWarning.arg("testnet"));
        config()->set(Config::warnOnTestnet, false);
    }

    // Beta
    if (config()->get(Config::warnOnAlpha).toBool()) {
        QString warning = "Feather Wallet is currently in beta.\n\nPlease report any bugs "
                          "you encounter on our Git repository, IRC or on /r/FeatherWallet.";
        this->showWarningMessageBox("Beta warning", warning);
        config()->set(Config::warnOnAlpha, false);
    }
}

void WindowManager::showWarningMessageBox(const QString &title, const QString &message) {
    QMessageBox msgBox;
    msgBox.setWindowIcon(icons()->icon("appicons/64x64.png"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(message);
    msgBox.setWindowTitle(title);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

// ######################## WALLET OPEN ########################

void WindowManager::tryOpenWallet(const QString &path, const QString &password) {
    // Path : path to .keys file

    QString absolutePath = path;
    if (absolutePath.startsWith("~")) {
        absolutePath.replace(0, 1, QDir::homePath());
    }

    // If the wallet is already open, just bring window to front
    for (const auto &window : m_windows) {
        if (absolutePath == window->walletKeysPath() || absolutePath == window->walletCachePath()) {
            window->bringToFront();
            return;
        }
    }

    if (!Utils::fileExists(path)) {
        this->handleWalletError(QString("Wallet not found: %1").arg(path));
        return;
    }

    m_openingWallet = true;
    m_walletManager->openWalletAsync(path, password, constants::networkType, 1);
}

void WindowManager::onWalletOpened(Wallet *wallet) {
    if (wallet->status() != Wallet::Status_Ok) {
        QString errMsg = wallet->errorString();
        if (wallet->status() == Wallet::Status_BadPassword) {
            // Don't show incorrect password when we try with empty password for the first time
            bool showIncorrectPassword = m_openWalletTriedOnce;
            m_openWalletTriedOnce = true;
            this->onWalletOpenPasswordRequired(showIncorrectPassword, wallet->cachePath());
        }
        else if (errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc")) {
            qCritical() << errMsg;
            WalletManager::clearWalletCache(wallet->cachePath()); // TODO: check this
            errMsg = QString("%1\n\nAttempted to clean wallet cache. Please restart Feather.").arg(errMsg);
            this->handleWalletError(errMsg);
        } else {
            this->handleWalletError(errMsg);
        }
        return;
    }

    this->onInitialNetworkConfigured();

    // Create new mainwindow with wallet

    m_splashDialog->hide();
    m_openWalletTriedOnce = false;
    auto *window = new MainWindow(this, wallet);
    m_windows.append(window);
    this->buildTrayMenu();
    m_openingWallet = false;
}

void WindowManager::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    QFileInfo fileInfo(path);

    PasswordDialog dialog{fileInfo.fileName(), invalidPassword};
    switch (dialog.exec()) {
        case QDialog::Rejected:
        {
            m_openWalletTriedOnce = false;
            this->showWizard(WalletWizard::Page_OpenWallet);
            return;
        }
    }

    this->tryOpenWallet(path, dialog.password);
}

bool WindowManager::autoOpenWallet() {
    QString autoPath = config()->get(Config::autoOpenWalletPath).toString();
    if (!autoPath.isEmpty() && autoPath.startsWith(QString::number(constants::networkType))) {
        autoPath.remove(0, 1);
    }
    if (!autoPath.isEmpty() && Utils::fileExists(autoPath)) {
        this->tryOpenWallet(autoPath, ""); // TODO: get password from --password
        return true;
    }
    return false;
}

// ######################## WALLET CREATION ########################

void WindowManager::tryCreateWallet(FeatherSeed seed, const QString &path, const QString &password,
                                    const QString &seedOffset) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        this->handleWalletError(err);
        return;
    }

    if (seed.mnemonic.isEmpty()) {
        this->handleWalletError("Mnemonic seed error. Failed to write wallet.");
        return;
    }

    Wallet *wallet = nullptr;
    if (seed.seedType == SeedType::TEVADOR) {
        wallet = m_walletManager->createDeterministicWalletFromSpendKey(path, password, seed.language, constants::networkType, seed.spendKey, seed.restoreHeight, constants::kdfRounds, seedOffset);
        wallet->setCacheAttribute("feather.seed", seed.mnemonic.join(" "));
        wallet->setCacheAttribute("feather.seedoffset", seedOffset);
    }
    if (seed.seedType == SeedType::MONERO) {
        wallet = m_walletManager->recoveryWallet(path, password, seed.mnemonic.join(" "), seedOffset, constants::networkType, seed.restoreHeight, constants::kdfRounds);
    }

    if (!wallet) {
        this->handleWalletError("Failed to write wallet");
        return;
    }

    this->onWalletOpened(wallet);
}

void WindowManager::tryCreateWalletFromDevice(const QString &path, const QString &password, int restoreHeight)
{
    if (Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        this->handleWalletError(err);
        return;
    }

    m_openingWallet = true;
    m_walletManager->createWalletFromDeviceAsync(path, password, constants::networkType, "Ledger", restoreHeight);
}

void WindowManager::tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address,
                                            const QString &viewkey, const QString &spendkey, quint64 restoreHeight) {
    if (Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        this->handleWalletError(err);
        return;
    }

    if (!WalletManager::addressValid(address, constants::networkType)) {
        auto err = QString("Failed to create wallet. Invalid address provided.").arg(path);
        this->handleWalletError(err);
        return;
    }

    if (!WalletManager::keyValid(viewkey, address, true, constants::networkType)) {
        auto err = QString("Failed to create wallet. Invalid viewkey provided.").arg(path);
        this->handleWalletError(err);
        return;
    }

    if (!spendkey.isEmpty() && !WalletManager::keyValid(spendkey, address, false, constants::networkType)) {
        auto err = QString("Failed to create wallet. Invalid spendkey provided.").arg(path);
        this->handleWalletError(err);
        return;
    }

    Wallet *wallet = m_walletManager->createWalletFromKeys(path, password, constants::seedLanguage, constants::networkType, address, viewkey, spendkey, restoreHeight);
    m_openingWallet = true;
    m_walletManager->walletOpened(wallet);
}

void WindowManager::onWalletCreated(Wallet *wallet) {
    // Currently only called when a wallet is created from device.
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        qDebug() << Q_FUNC_INFO << QString("Wallet open error: %1").arg(wallet->errorString());
        this->displayWalletErrorMessage(wallet->errorString());
        m_splashDialog->hide();
        this->showWizard(WalletWizard::Page_Menu);
        return;
    }

    this->onWalletOpened(wallet);
}

// ######################## ERROR HANDLING ########################

void WindowManager::handleWalletError(const QString &message) {
    qCritical() << message;
    this->displayWalletErrorMessage(message);
    this->initWizard();
}

void WindowManager::displayWalletErrorMessage(const QString &message) {
    QString errMsg = message;
    if (message.contains("No device found")) {
        errMsg += "\n\nThis wallet is backed by a hardware device. Make sure the Monero app is opened on the device.\n"
                  "You may need to restart Feather before the device can get detected.";
    }
    if (message.contains("Unable to open device")) {
        errMsg += "\n\nThe device might be in use by a different application.";
#if defined(Q_OS_LINUX)
        errMsg += "\n\nNote: On Linux you may need to follow the instructions in the link below before the device can be opened:\n"
                  "<a>https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues</a>";
#endif
    }

    if (message.contains("SW_CLIENT_NOT_SUPPORTED")) {
        errMsg += "\n\nIncompatible version: you may need to upgrade the Monero app on the Ledger device to the latest version.";
    }
    else if (message.contains("Wrong Device Status")) {
        errMsg += "\n\nThe device may need to be unlocked.";
    }
    else if (message.contains("Wrong Channel")) {
        errMsg += "\n\nRestart the hardware device and try again.";
    }

    QMessageBox msgBox;
    msgBox.setWindowIcon(icons()->icon("appicons/64x64.png"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(errMsg);
    msgBox.setWindowTitle("Wallet error");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

// ######################## DEVICE ########################

void WindowManager::onDeviceButtonRequest(quint64 code) {
    m_splashDialog->setMessage("Action required on device: Export the view key to open the wallet.");
    m_splashDialog->setIcon(QPixmap(":/assets/images/key.png"));
    m_splashDialog->show();
    m_splashDialog->setEnabled(true);
}

void WindowManager::onDeviceError(const QString &errorMessage) {
    // TODO: when does this get called?
    qCritical() << Q_FUNC_INFO << errorMessage;
}

// ######################## TRAY ########################

void WindowManager::buildTrayMenu() {
    QMenu *menu;
    if (!m_tray->contextMenu()) {
        menu = new QMenu();
        m_tray->setContextMenu(menu);
    } else {
        menu = m_tray->contextMenu();
        menu->clear();
    }

    for (const auto &window : m_windows) {
        QString name = window->walletName();
        QMenu *submenu = menu->addMenu(name);
        submenu->addAction("Show/Hide", window, &MainWindow::showOrHide);
        submenu->addAction("Close", window, &MainWindow::close);
    }
    menu->addSeparator();
    menu->addAction("Exit Feather", this, &WindowManager::close);
}

// ######################## NETWORKING ########################

void WindowManager::onInitialNetworkConfigured() {
    if (!m_initialNetworkConfigured) {
        m_initialNetworkConfigured = true;
        appData();
        this->initTor();
        this->initWS();
    }
}

void WindowManager::initTor() {
    torManager()->init();
    torManager()->start();

    connect(torManager(), &TorManager::connectionStateChanged, &websocketNotifier()->websocketClient, &WebsocketClient::onToggleConnect);

    this->onTorSettingsChanged();
}

void WindowManager::onTorSettingsChanged() {
    if (Utils::isTorsocks()) {
        return;
    }

    // use local tor -> bundled tor
    QString host = config()->get(Config::socks5Host).toString();
    quint16 port = config()->get(Config::socks5Port).toString().toUShort();
    if (!torManager()->isLocalTor()) {
        host = torManager()->featherTorHost;
        port = torManager()->featherTorPort;
    }

    QNetworkProxy proxy{QNetworkProxy::Socks5Proxy, host, port};
    getNetworkTor()->setProxy(proxy);
    websocketNotifier()->websocketClient.webSocket.setProxy(proxy);

    emit torSettingsChanged();
}

void WindowManager::initWS() {
    websocketNotifier()->websocketClient.start();
}

// ######################## WIZARD ########################

WalletWizard* WindowManager::createWizard(WalletWizard::Page startPage) const {
    auto *wizard = new WalletWizard;
    connect(wizard, &WalletWizard::initialNetworkConfigured, this, &WindowManager::onInitialNetworkConfigured);
    connect(wizard, &WalletWizard::skinChanged, this, &WindowManager::changeSkin);
    connect(wizard, &WalletWizard::openWallet, this, &WindowManager::tryOpenWallet);
    connect(wizard, &WalletWizard::createWallet, this, &WindowManager::tryCreateWallet);
    connect(wizard, &WalletWizard::createWalletFromKeys, this, &WindowManager::tryCreateWalletFromKeys);
    connect(wizard, &WalletWizard::createWalletFromDevice, this, &WindowManager::tryCreateWalletFromDevice);
    return wizard;
}

void WindowManager::initWizard() {
    auto startPage = WalletWizard::Page_Menu;
    if (config()->get(Config::firstRun).toBool() && !(TailsOS::detect() || WhonixOS::detect())) {
        startPage = WalletWizard::Page_Network;
    }

    this->showWizard(startPage);
}

void WindowManager::showWizard(WalletWizard::Page startPage) {
    if (!m_wizard) {
        m_wizard = this->createWizard(startPage);
    }

    m_wizard->setStartId(startPage);
    m_wizard->restart();
    m_wizard->setEnabled(true);
    m_wizard->show();
}

void WindowManager::wizardOpenWallet() {
    this->showWizard(WalletWizard::Page_OpenWallet);
}

// ######################## SKINS ########################

void WindowManager::initSkins() {
    m_skins.insert("Native", "");

    QString qdarkstyle = this->loadStylesheet(":qdarkstyle/style.qss");
    if (!qdarkstyle.isEmpty())
        m_skins.insert("QDarkStyle", qdarkstyle);

    QString breeze_dark = this->loadStylesheet(":/dark.qss");
    if (!breeze_dark.isEmpty())
        m_skins.insert("Breeze/Dark", breeze_dark);

    QString breeze_light = this->loadStylesheet(":/light.qss");
    if (!breeze_light.isEmpty())
        m_skins.insert("Breeze/Light", breeze_light);

    QString skin = config()->get(Config::skin).toString();
    qApp->setStyleSheet(m_skins[skin]);
}

QString WindowManager::loadStylesheet(const QString &resource) {
    QFile f(resource);
    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
        f.close();
        return "";
    }

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    QString data = ts.readAll();
    f.close();

    return data;
}

void WindowManager::changeSkin(const QString &skinName) {
    if (!m_skins.contains(skinName)) {
        qWarning() << QString("No such skin %1").arg(skinName);
        return;
    }

    config()->set(Config::skin, skinName);
    qApp->setStyleSheet(m_skins[skinName]);
    qDebug() << QString("Skin changed to %1").arg(skinName);
}
