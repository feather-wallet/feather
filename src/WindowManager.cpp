// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "WindowManager.h"

#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QWindow>
#include <QNetworkProxy>

#include "Application.h"
#include "constants.h"
#include "MainWindow.h"
#include "dialog/DocsDialog.h"
#include "dialog/PasswordDialog.h"
#include "dialog/SplashDialog.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "utils/Icons.h"
#include "utils/NetworkManager.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"
#include "utils/AppData.h"

WindowManager::WindowManager(QObject *parent)
    : QObject(parent)
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

    connect(qApp, SIGNAL(anotherInstanceStarted()), this, SLOT(raise()));
    connect(qApp, &QGuiApplication::lastWindowClosed, this, &WindowManager::quitAfterLastWindow);

    m_tray = new QSystemTrayIcon(icons()->icon("appicons/64x64.png"));
    m_tray->setToolTip("Feather Wallet");
    this->buildTrayMenu();
    m_tray->setVisible(conf()->get(Config::showTrayIcon).toBool());

    this->initSkins();
    this->patchMacStylesheet();

    this->showCrashLogs();

    if (!conf()->get(Config::firstRun).toBool() || TailsOS::detect() || WhonixOS::detect()) {
        this->onInitialNetworkConfigured();
    }

    this->startupWarning();

    if (!this->autoOpenWallet()) {
        this->initWizard();
    }
}

QPointer<WindowManager> WindowManager::m_instance(nullptr);

void WindowManager::setEventFilter(EventFilter *ef) {
    eventFilter = ef;
}

WindowManager::~WindowManager() {
    qDebug() << "~WindowManager";
    m_cleanupThread->quit();
    m_cleanupThread->wait();
    qDebug() << "WindowManager: cleanup thread done" << QThread::currentThreadId();
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
    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    for (const auto &window: m_windows) {
        window->close();
    }

    if (m_splashDialog) {
        m_splashDialog->deleteLater();
    }
    if (m_tray) {
        m_tray->deleteLater();
    }
    if (m_wizard) {
        m_wizard->deleteLater();
    }
    if (m_docsDialog) {
        m_docsDialog->deleteLater();
    }

    deleteLater();

    qDebug() << "Calling QApplication::quit()";
    QApplication::quit();
}

void WindowManager::closeWindow(MainWindow *window) {
    qDebug() << "WindowManager: closing Window";
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
    if (constants::networkType == NetworkType::STAGENET && conf()->get(Config::warnOnStagenet).toBool()) {
        this->showWarningMessageBox("Warning", worthlessWarning.arg("stagenet"));
        conf()->set(Config::warnOnStagenet, false);
    }
    else if (constants::networkType == NetworkType::TESTNET && conf()->get(Config::warnOnTestnet).toBool()){
        this->showWarningMessageBox("Warning", worthlessWarning.arg("testnet"));
        conf()->set(Config::warnOnTestnet, false);
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
        emit preferredFiatCurrencyChanged();
    });
    connect(&settings, &Settings::skinChanged, this, &WindowManager::onChangeTheme);
    connect(&settings, &Settings::updateBalance, this, &WindowManager::updateBalance);
    connect(&settings, &Settings::proxySettingsChanged, this, &WindowManager::onProxySettingsChanged);
    connect(&settings, &Settings::offlineMode, this, &WindowManager::offlineMode);
    connect(&settings, &Settings::manualFeeSelectionEnabled, this, &WindowManager::manualFeeSelectionEnabled);
    connect(&settings, &Settings::subtractFeeFromAmountEnabled, this, &WindowManager::subtractFeeFromAmountEnabled);
    connect(&settings, &Settings::hideUpdateNotifications, [this](bool hidden){
        for (const auto &window : m_windows) {
            window->onHideUpdateNotifications(hidden);
        }
    });
    connect(&settings, &Settings::showTrayIcon, [this](bool visible) {
        m_tray->setVisible(visible);
    });

    if (showProxyTab) {
        settings.showNetworkProxyTab();
    }

    settings.exec();
}

// ######################## DOCS ########################

void WindowManager::showDocs(QObject *parent, const QString &doc, bool modal) {
    if (!m_docsDialog) {
        m_docsDialog = new DocsDialog();
    }

    m_docsDialog->setModal(modal);

    m_docsDialog->show();
    if (m_docsDialog->isMinimized()) {
        m_docsDialog->showNormal();
    }
    m_docsDialog->raise();
    m_docsDialog->activateWindow();

    QWindow *window = Utils::windowForQObject(parent);
    if (window != nullptr) {
        QPoint centerOfParent = window->frameGeometry().center();
        m_docsDialog->move(centerOfParent.x() - m_docsDialog->width() / 2, centerOfParent.y() - m_docsDialog->height() / 2);
    } else {
        qDebug() << "Unable to center docs window";
    }

    if (!doc.isEmpty()) {
        m_docsDialog->showDoc(doc);
    }
}

void WindowManager::setDocsHighlight(const QString &highlight) {
    if (!m_docsDialog) {
        return;
    }

    m_docsDialog->updateHighlights(highlight);
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
        this->handleWalletError({nullptr, Utils::ERROR, "Unable to open wallet", QString("Wallet not found: %1").arg(path)});
        return;
    }

    m_openingWallet = true;
    m_walletManager->openWalletAsync(path, password, constants::networkType, constants::kdfRounds, Utils::ringDatabasePath());
}

void WindowManager::onWalletOpened(Wallet *wallet) {
    if (!wallet) {
        this->handleWalletError({nullptr, Utils::ERROR, "Unable to open wallet", "This should never happen. If you encounter this error, please report it to the developers.", {}, "report_an_issue"});
        return;
    }

    auto status = wallet->status();
    if (status != Wallet::Status_Ok) {
        QString errMsg = wallet->errorString();
        wallet->deleteLater();
        if (status == Wallet::Status_BadPassword) {
            // Don't show incorrect password when we try with empty password for the first time
            bool showIncorrectPassword = m_openWalletTriedOnce;
            m_openWalletTriedOnce = true;
            this->onWalletOpenPasswordRequired(showIncorrectPassword, wallet->keysPath());
            return; // Do not remove this
        }
        else if (errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc") || errMsg == "invalid signature") {
            qCritical() << errMsg;
            WalletManager::clearWalletCache(wallet->cachePath());
            errMsg = QString("%1\n\nWallet cache is unusable, moving it.").arg(errMsg);
            this->handleWalletError({nullptr, Utils::ERROR, "Unable to open wallet", errMsg, {"Try opening this wallet again.", "If this keeps happening, please file a bug report."}, "report_an_issue"});
        }
        else if (errMsg.startsWith("failed to read file")) {
#if defined(Q_OS_MACOS)
            Utils::Message message{nullptr, Utils::ERROR, "Unable to open wallet", errMsg, {"You may need to give Feather permission to access the folder", "In the System Settings app, go to 'Privacy & Security' -> 'Files & Folders'"}};
#else
            Utils::Message message{nullptr, Utils::ERROR, "Unable to open wallet", errMsg, {"You may need to change the permissions on the wallet directory."}};
#endif
            this->handleWalletError(message);
        }
        else {
            Utils::Message message{nullptr, Utils::ERROR, "Unable to open wallet"};
            this->handleDeviceError(errMsg, message);
            this->handleWalletError(message);
        }

        m_openingWallet = false;
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
            m_openingWallet = false;
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
    QString autoPath = conf()->get(Config::autoOpenWalletPath).toString();
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
    if (Utils::fileExists(path)) {
        this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", QString("File already exists: %1").arg(path)});
        return;
    }

    if (seed.mnemonic.isEmpty()) {
        this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", "Mnemonic seed is empty"});
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
        this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet"});
        return;
    }

    wallet->setCacheAttribute("feather.seed", seed.mnemonic.join(" "));
    wallet->setCacheAttribute("feather.seedoffset", seedOffset);
    // Store attributes now, so we don't lose them on crash / forced exit
    wallet->store();

    if (newWallet) {
        wallet->setNewWallet();
    }

    this->onWalletOpened(wallet);
}

void WindowManager::tryCreateWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight, const QString &subaddressLookahead)
{
    if (Utils::fileExists(path)) {
        this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet from device", QString("File already exists: %1").arg(path)});
        return;
    }

    m_openingWallet = true;
    m_walletManager->createWalletFromDeviceAsync(path, password, constants::networkType, deviceName, restoreHeight, subaddressLookahead);
}

void WindowManager::tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address,
                                            const QString &viewkey, const QString &spendkey, quint64 restoreHeight, const QString &subaddressLookahead) {
    if (Utils::fileExists(path)) {
        this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", QString("File already exists: %1").arg(path)});
        return;
    }

    Wallet *wallet;
    if (address.isEmpty() && viewkey.isEmpty() && !spendkey.isEmpty()) {
        wallet = m_walletManager->createDeterministicWalletFromSpendKey(path, password, constants::seedLanguage, constants::networkType, spendkey, restoreHeight, constants::kdfRounds, "", subaddressLookahead);
    }
    else {
        if (!spendkey.isEmpty() && !WalletManager::keyValid(spendkey, address, false, constants::networkType)) {
            this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", "Invalid spendkey provided"});
            return;
        }

        if (!WalletManager::addressValid(address, constants::networkType)) {
            this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", "Invalid address provided"});
            return;
        }

        if (!WalletManager::keyValid(viewkey, address, true, constants::networkType)) {
            this->handleWalletError({nullptr, Utils::ERROR, "Failed to create wallet", "Invalid viewkey provided"});
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
        QString error = wallet->errorString();
        Utils::Message message{m_wizard, Utils::ERROR, "Failed to create wallet from device"};
        this->handleDeviceError(error, message);
        this->showWizard(WalletWizard::Page_Menu);
        Utils::showMsg(message);
        m_splashDialog->hide();
        m_openingWallet = false;
        return;
    }

    this->onWalletOpened(wallet);
}

// ######################## ERROR HANDLING ########################

void WindowManager::handleWalletError(const Utils::Message &message) {
    Utils::showMsg(message);
    this->initWizard();
}

void WindowManager::handleDeviceError(const QString &error, Utils::Message &msg) {
    msg.description = error;

    // Ledger
    if (error.contains("No device found")) {
        msg.description = "No Ledger device found.";
        msg.helpItems = {"Make sure the Monero app is open on the device.", "If the problem persists, try restarting Feather."};
        msg.doc = "create_wallet_hardware_device";
    }
    else if (error.contains("Unable to open device")) {
        msg.description = "Unable to open device.";
        msg.helpItems = {"The device might be in use by a different application."};
#if defined(Q_OS_LINUX)
        msg.helpItems.append("On Linux you may need to follow the instructions in the link below before the device can be opened:\n"
                         "https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues");
        msg.link = "https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues";
#endif
    }

    // Trezor
    else if (error.contains("Unable to claim libusb device")) {
        msg.description = "Unable to claim Trezor device";
        msg.helpItems = {"Please make sure the device is not used by another program, like Trezor Suite or trezord, and try again."};
    }
    else if (error.contains("Cannot get a device address")) {
        msg.description = "Cannot get a device address";
        msg.helpItems = {"Reattach the Trezor device and try again"};
    }
    else if (error.contains("Could not connect to the device Trezor") || error.contains("Device connect failed")) {
        msg.description = "Could not connect to the Trezor device";
        msg.helpItems = {"Make sure the device is connected to your computer and unlocked."};
#if defined(Q_OS_LINUX)
        msg.helpItems.append("On Linux you may need to follow the instructions in the link below before the device can be opened:\n"
                         "https://wiki.trezor.io/Udev_rules");
        msg.link = "https://wiki.trezor.io/Udev_rules";
#endif
    }
    else if (error.contains("Failed to acquire device")) {
        msg.helpItems = {"Make sure Trezor Suite and trezord are closed."};
    }
    else if (error.contains("SW_CLIENT_NOT_SUPPORTED")) {
        msg.helpItems = {"Upgrade your Ledger device firmware to the latest version using Ledger Live.\n"
                     "Then upgrade the Monero app for the Ledger device to the latest version."};
    }
    else if (error.contains("Wrong Device Status")) {
        msg.helpItems = {"The device may need to be unlocked."};
    }
    else if (error.contains("Wrong Channel")) {
        msg.helpItems = {"Restart the hardware device and try again."};
    }
    else {
        msg.doc = "report_an_issue";
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

void WindowManager::onDeviceError(const QString &errorMessage, quint64 errorCode) {
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

    if (conf()->get(Config::hideNotifications).toBool()) {
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

    QNetworkProxy proxy{QNetworkProxy::NoProxy};
    if (conf()->get(Config::proxy).toInt() != Config::Proxy::None) {
        QString host = conf()->get(Config::socks5Host).toString();
        quint16 port = conf()->get(Config::socks5Port).toString().toUShort();

        proxy = QNetworkProxy{QNetworkProxy::Socks5Proxy, host, port};
        getNetworkSocks5()->setProxy(proxy);
    }

    qWarning() << "Proxy: " << proxy.hostName() << " " << proxy.port();

    emit proxySettingsChanged();
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
    if (conf()->get(Config::firstRun).toBool() && !(TailsOS::detect() || WhonixOS::detect())) {
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

    QString skin = conf()->get(Config::skin).toString();
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

    conf()->set(Config::skin, skinName);

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

WindowManager* WindowManager::instance()
{
    if (!m_instance) {
        m_instance = new WindowManager(QCoreApplication::instance());
    }

    return m_instance;
}