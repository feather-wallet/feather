// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WindowManager.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>

#include "constants.h"
#include "dialog/PasswordDialog.h"
#include "dialog/SplashDialog.h"
#include "utils/Icons.h"
#include "utils/NetworkManager.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"
#include "utils/TorManager.h"
#include "utils/WebsocketNotifier.h"

WindowManager::WindowManager(QObject *parent, EventFilter *eventFilter)
    : QObject(parent)
    , eventFilter(eventFilter)
{
    m_walletManager = WalletManager::instance();
    m_splashDialog = new SplashDialog();
    m_cleanupThread = new QThread(this);

    connect(m_walletManager, &WalletManager::walletOpened,        this, &WindowManager::onWalletOpened);
    connect(m_walletManager, &WalletManager::walletCreated,       this, &WindowManager::onWalletCreated);
    connect(m_walletManager, &WalletManager::deviceButtonRequest, this, &WindowManager::onDeviceButtonRequest);
    connect(m_walletManager, &WalletManager::deviceButtonPressed, this, &WindowManager::onDeviceButtonPressed);
    connect(m_walletManager, &WalletManager::deviceError,         this, &WindowManager::onDeviceError);
    connect(m_walletManager, &WalletManager::walletPassphraseNeeded, this, &WindowManager::onWalletPassphraseNeeded);

    connect(qApp, &QGuiApplication::lastWindowClosed, this, &WindowManager::quitAfterLastWindow);

    m_tray = new QSystemTrayIcon(icons()->icon("appicons/64x64.png"));
    m_tray->setToolTip("Feather Wallet");
    this->buildTrayMenu();
    m_tray->show();

    this->initSkins();
    this->patchMacStylesheet();

    this->showCrashLogs();

    if (!config()->get(Config::firstRun).toBool() || TailsOS::detect() || WhonixOS::detect()) {
        this->onInitialNetworkConfigured();
    }

    this->startupWarning();

    if (!this->autoOpenWallet()) {
        this->initWizard();
    }
}

WindowManager::~WindowManager() {
    qDebug() << "~WindowManager";
    m_cleanupThread->quit();
    m_cleanupThread->wait();
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

    m_wizard->deleteLater();
    m_splashDialog->deleteLater();
    m_tray->deleteLater();

    torManager()->stop();

    QApplication::quit();
}

void WindowManager::closeWindow(MainWindow *window) {
    qDebug() << "closing Window";
    m_windows.removeOne(window);

    // Move Wallet to a different thread for cleanup, so it doesn't block GUI thread
    window->m_wallet->moveToThread(m_cleanupThread);
    m_cleanupThread->start();
    window->m_wallet->deleteLater();

    window->deleteLater();
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

void WindowManager::raise() {
    if (!m_windows.isEmpty()) {
        m_windows.first()->bringToFront();
    }
    else if (m_wizard) {
        m_wizard->show();
        m_wizard->raise();
        m_wizard->activateWindow();
    }
    else {
        // This shouldn't happen
        this->close();
    }
}

// ######################## SETTINGS ########################

void WindowManager::showSettings(Nodes *nodes, QWidget *parent, bool showProxyTab) {
    Settings settings{nodes, parent};

    connect(&settings, &Settings::preferredFiatCurrencyChanged, [this]{
        for (const auto &window : m_windows) {
            window->onPreferredFiatCurrencyChanged();
        }
    });
    connect(&settings, &Settings::skinChanged, this, &WindowManager::onChangeTheme);
    connect(&settings, &Settings::updateBalance, this, &WindowManager::updateBalance);
    connect(&settings, &Settings::proxySettingsChanged, this, &WindowManager::onProxySettingsChanged);
    connect(&settings, &Settings::websocketStatusChanged, this, &WindowManager::onWebsocketStatusChanged);
    connect(&settings, &Settings::offlineMode, this, &WindowManager::offlineMode);
    connect(&settings, &Settings::hideUpdateNotifications, [this](bool hidden){
        for (const auto &window : m_windows) {
            window->onHideUpdateNotifications(hidden);
        }
    });

    if (showProxyTab) {
        settings.showNetworkProxyTab();
    }

    settings.exec();
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
    m_walletManager->openWalletAsync(path, password, constants::networkType, constants::kdfRounds, Utils::ringDatabasePath());
}

void WindowManager::onWalletOpened(Wallet *wallet) {
    if (!wallet) {
        QString err{"Unable to open wallet"};
        this->handleWalletError(err);
        return;
    }

    auto status = wallet->status();
    if (status != Wallet::Status_Ok) {
        QString errMsg = wallet->errorString();
        QString keysPath = wallet->keysPath();
        QString cachePath = wallet->cachePath();
        wallet->deleteLater();
        if (status == Wallet::Status_BadPassword) {
            // Don't show incorrect password when we try with empty password for the first time
            bool showIncorrectPassword = m_openWalletTriedOnce;
            m_openWalletTriedOnce = true;
            this->onWalletOpenPasswordRequired(showIncorrectPassword, keysPath);
        }
        else if (errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc")) {
            qCritical() << errMsg;
            WalletManager::clearWalletCache(cachePath);
            errMsg = QString("%1\n\nAttempted to clean wallet cache. Please restart Feather.").arg(errMsg);
            this->handleWalletError(errMsg);
        } else {
            this->handleWalletError(errMsg);
        }
        return;
    }

    this->onInitialNetworkConfigured();

//    if (!wallet->cacheAttributeExists("feather.xmrig_password") && !wallet->cacheAttributeExists("feather.created")) {
//        auto result = QMessageBox::question(nullptr, "Foreign wallet",
//                                            "This wallet file was not created with Feather. This may cause unexpected behavior. Please restore your wallet from seed.\n\nOpen this wallet anyway?");
//        if (result == QMessageBox::No) {
//            wallet->deleteLater();
//            this->initWizard();
//            return;
//        }
//    }

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
            if (m_wizard) {
                m_wizard->show();
            } else {
                this->showWizard(WalletWizard::Page_Menu);
            }
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

void WindowManager::tryCreateWallet(Seed seed, const QString &path, const QString &password, const QString &seedLanguage,
                                    const QString &seedOffset, const QString &subaddressLookahead, bool newWallet) {
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
    if (seed.type == Seed::Type::POLYSEED || seed.type == Seed::Type::TEVADOR) {
        wallet = m_walletManager->createDeterministicWalletFromSpendKey(path, password, seed.language, constants::networkType, seed.spendKey, seed.restoreHeight, constants::kdfRounds, seedOffset, subaddressLookahead);
    }
    else if (seed.type == Seed::Type::MONERO) {
        wallet = m_walletManager->recoveryWallet(path, password, seed.mnemonic.join(" "), seedOffset, constants::networkType, seed.restoreHeight, constants::kdfRounds);
    }

    if (!wallet) {
        this->handleWalletError("Failed to write wallet");
        return;
    }

    wallet->setCacheAttribute("feather.seed", seed.mnemonic.join(" "));
    wallet->setCacheAttribute("feather.seedoffset", seedOffset);

    if (newWallet) {
        wallet->setNewWallet();
    }

    this->onWalletOpened(wallet);
}

void WindowManager::tryCreateWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight, const QString &subaddressLookahead)
{
    if (Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        this->handleWalletError(err);
        return;
    }

    m_openingWallet = true;
    m_walletManager->createWalletFromDeviceAsync(path, password, constants::networkType, deviceName, restoreHeight, subaddressLookahead);
}

void WindowManager::tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address,
                                            const QString &viewkey, const QString &spendkey, quint64 restoreHeight, const QString &subaddressLookahead) {
    if (Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        this->handleWalletError(err);
        return;
    }

    Wallet *wallet;
    if (address.isEmpty() && viewkey.isEmpty() && !spendkey.isEmpty()) {
        wallet = m_walletManager->createDeterministicWalletFromSpendKey(path, password, constants::seedLanguage, constants::networkType, spendkey, restoreHeight, constants::kdfRounds, "", subaddressLookahead);
    }
    else {
        if (!spendkey.isEmpty() && !WalletManager::keyValid(spendkey, address, false, constants::networkType)) {
            auto err = QString("Failed to create wallet. Invalid spendkey provided.").arg(path);
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

        wallet = m_walletManager->createWalletFromKeys(path, password, constants::seedLanguage, constants::networkType, address, viewkey, spendkey, restoreHeight, constants::kdfRounds, subaddressLookahead);
    }

    m_openingWallet = true;
    this->onWalletOpened(wallet);
}

void WindowManager::onWalletCreated(Wallet *wallet) {
    // Currently only called when a wallet is created from device.
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        qDebug() << Q_FUNC_INFO << QString("Wallet open error: %1").arg(wallet->errorString());
        this->displayWalletErrorMessage(wallet->errorString());
        m_splashDialog->hide();
        this->showWizard(WalletWizard::Page_Menu);
        m_openingWallet = false;
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
    QString errMsg = QString("Error: %1").arg(message);
    QString link;

    // Ledger
    if (message.contains("No device found")) {
        errMsg += "\n\nThis wallet is backed by a Ledger hardware device. Make sure the Monero app is opened on the device.\n"
                  "You may need to restart Feather before the device can get detected.";
    }
    if (message.contains("Unable to open device")) {
        errMsg += "\n\nThe device might be in use by a different application.";
#if defined(Q_OS_LINUX)
        errMsg += "\n\nNote: On Linux you may need to follow the instructions in the link below before the device can be opened:\n"
                  "https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues";
        link = "https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues";
#endif
    }

    // TREZOR
    if (message.contains("Unable to claim libusb device")) {
        errMsg += "\n\nThis wallet is backed by a Trezor hardware device. Feather was unable to access the device. "
                  "Please make sure it is not opened by another program and try again.";
    }
    if (message.contains("Cannot get a device address")) {
        errMsg += "\n\nRestart the Trezor hardware device and try again.";
    }

    if (message.contains("Could not connect to the device Trezor") || message.contains("Device connect failed")) {
        errMsg += "\n\nThis wallet is backed by a Trezor hardware device. Make sure the device is connected to your computer and unlocked.";
#if defined(Q_OS_LINUX)
        errMsg += "\n\nNote: On Linux you may need to follow the instructions in the link below before the device can be opened:\n"
                  "https://wiki.trezor.io/Udev_rules";
        link = "https://wiki.trezor.io/Udev_rules";
#endif
    }

    if (message.contains("SW_CLIENT_NOT_SUPPORTED")) {
        errMsg += "\n\nIncompatible version: upgrade your Ledger device firmware to the latest version using Ledger Live.\n"
                  "Then upgrade the Monero app for the Ledger device to the latest version.";
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
    QPushButton *openLinkButton = nullptr;
    if (!link.isEmpty()) {
        openLinkButton = msgBox.addButton("Open link", QMessageBox::ActionRole);
    }
    msgBox.exec();
    if (openLinkButton && msgBox.clickedButton() == openLinkButton) {
        Utils::externalLinkWarning(nullptr, link);
    }
}

void WindowManager::showCrashLogs() {
    QString crashLogPath{Config::defaultConfigDir().path() + "/crash_report.txt"};
    QFile crashLogFile{crashLogPath};

    if (!crashLogFile.exists()) {
        return;
    }

    bool r = crashLogFile.open(QIODevice::ReadOnly);
    if (!r) {
        qWarning() << "Unable to open crash log file: " << crashLogPath;
        return;
    }

    QTextStream log(&crashLogFile);
    QString logString = log.readAll();
    crashLogFile.close();

    bool renamed = false;
    for (int i = 1; i < 999; i++) {
        QString name{QString("/crash_report_%1.txt").arg(QString::number(i))};
        if (crashLogFile.rename(Config::defaultConfigDir().path() + name)) {
            renamed = true;
            break;
        }
    }

    if (!renamed) {
        crashLogFile.remove();
    }

    QDialog dialog(nullptr);
    dialog.setWindowTitle("Crash report");

    QVBoxLayout layout;
    QLabel msg{"Feather encountered an unrecoverable error.\n\nPlease send a copy of these logs to the developers. Logs are not automatically reported.\n"};
    QTextEdit logs;
    logs.setText(logString);

    layout.addWidget(&msg);
    layout.addWidget(&logs);
    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();

    exit(0);
}

// ######################## DEVICE ########################

void WindowManager::onDeviceButtonRequest(quint64 code) {
    QString message;
    switch (code) {
        case 1: // Trezor
            message = "Action required on device: enter your PIN to continue.";
            break;
        case 8: // Trezor
            message = "Action required on device: Export watch-only credentials to open the wallet.";
            break;
        case 19: // Trezor
            message = "Action required on device: Enter passphrase to open the wallet.";
            break;
        default:
            message = "Action required on device: Export the view key to open the wallet.";
    }

    m_splashDialog->setMessage(message);
    m_splashDialog->setIcon(QPixmap(":/assets/images/key.png"));
    m_splashDialog->show();
    m_splashDialog->setEnabled(true);
}

void WindowManager::onDeviceButtonPressed() {
    m_splashDialog->hide();
}

void WindowManager::onDeviceError(const QString &errorMessage) {
    // TODO: when does this get called?
    qCritical() << Q_FUNC_INFO << errorMessage;
}

void WindowManager::onWalletPassphraseNeeded(bool on_device) {
    auto button = QMessageBox::question(nullptr, "Wallet Passphrase Needed", "Enter passphrase on hardware wallet?\n\n"
                                                                             "It is recommended to enter passphrase on "
                                                                             "the hardware wallet for better security.",
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (button == QMessageBox::Yes) {
        m_walletManager->onPassphraseEntered("", true, false);
        return;
    }

    bool ok;
    QString passphrase = QInputDialog::getText(nullptr, "Wallet Passphrase Needed", "Enter passphrase:", QLineEdit::EchoMode::Password, "", &ok);
    m_walletManager->onPassphraseEntered(passphrase, false, false);
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

void WindowManager::notify(const QString &title, const QString &message, int duration) {
    if (!m_tray || !QSystemTrayIcon::supportsMessages()) {
        return;
    }

    if (config()->get(Config::hideNotifications).toBool()) {
        return;
    }

    m_tray->showMessage(title, message, icons()->icon("appicons/64x64.png"), duration);
}

// ######################## NETWORKING ########################

void WindowManager::onInitialNetworkConfigured() {
    if (!m_initialNetworkConfigured) {
        m_initialNetworkConfigured = true;
        appData();

        this->onProxySettingsChanged();
    }
}

void WindowManager::onProxySettingsChanged() {
    if (Utils::isTorsocks()) {
        return;
    }

    // Will kill the process if necessary
    torManager()->init();
    torManager()->start();

    QNetworkProxy proxy{QNetworkProxy::NoProxy};
    if (config()->get(Config::proxy).toInt() != Config::Proxy::None) {
        QString host = config()->get(Config::socks5Host).toString();
        quint16 port = config()->get(Config::socks5Port).toString().toUShort();

        if (config()->get(Config::proxy).toInt() == Config::Proxy::Tor && !torManager()->isLocalTor()) {
            host = torManager()->featherTorHost;
            port = torManager()->featherTorPort;
        }

        proxy = QNetworkProxy{QNetworkProxy::Socks5Proxy, host, port};
        getNetworkSocks5()->setProxy(proxy);
    }

    qWarning() << "Proxy: " << proxy.hostName() << " " << proxy.port();

    // Switch websocket to new proxy and update URL
    websocketNotifier()->websocketClient->stop();
    websocketNotifier()->websocketClient->webSocket->setProxy(proxy);
    websocketNotifier()->websocketClient->nextWebsocketUrl();
    websocketNotifier()->websocketClient->restart();

    emit proxySettingsChanged();
}

void WindowManager::onWebsocketStatusChanged(bool enabled) {
    emit websocketStatusChanged(enabled);
}

// ######################## WIZARD ########################

WalletWizard* WindowManager::createWizard(WalletWizard::Page startPage) {
    auto *wizard = new WalletWizard;
    connect(wizard, &WalletWizard::initialNetworkConfigured, this, &WindowManager::onInitialNetworkConfigured);
    connect(wizard, &WalletWizard::showSettings, [this, wizard]{
        this->showSettings(nullptr, wizard);
    });
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

    m_wizard->resetFields();
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

void WindowManager::onChangeTheme(const QString &skinName) {
    if (!m_skins.contains(skinName)) {
        qWarning() << QString("No such skin %1").arg(skinName);
        return;
    }

    config()->set(Config::skin, skinName);

    qApp->setStyleSheet(m_skins[skinName]);
    qDebug() << QString("Skin changed to %1").arg(skinName);

    this->patchMacStylesheet();

    for (const auto &window : m_windows) {
        window->skinChanged(skinName);
    }
}

void WindowManager::patchMacStylesheet() {
#if defined(Q_OS_MACOS)
    QString styleSheet = qApp->styleSheet();

    auto patch = Utils::fileOpenQRC(":assets/macStylesheet.patch");
    auto patch_text = Utils::barrayToString(patch);
    styleSheet += patch_text;

    qApp->setStyleSheet(styleSheet);
#endif
}