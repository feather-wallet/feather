// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QMessageBox>
#include <QCoreApplication>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config-feather.h"
#include "dialog/txconfdialog.h"
#include "dialog/txconfadvdialog.h"
#include "dialog/debuginfodialog.h"
#include "dialog/walletinfodialog.h"
#include "dialog/torinfodialog.h"
#include "dialog/viewonlydialog.h"
#include "dialog/broadcasttxdialog.h"
#include "dialog/tximportdialog.h"
#include "dialog/passworddialog.h"
#include "dialog/balancedialog.h"
#include "dialog/WalletCacheDebugDialog.h"
#include "dialog/UpdateDialog.h"
#include "globals.h"
#include "libwalletqt/AddressBook.h"
#include "utils/AsyncTask.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/SemanticVersion.h"
#include "utils/NetworkManager.h"
#include "utils/Icons.h"
#include "utils/WebsocketNotifier.h"
#include "utils/Updater.h"

MainWindow * MainWindow::pMainWindow = nullptr;

MainWindow::MainWindow(AppContext *ctx, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ctx(ctx)
{
    pMainWindow = this;
    ui->setupUi(this);

    m_windowSettings = new Settings(this);
    m_windowCalc = new CalcWindow(this);
    m_splashDialog = new SplashDialog(this);

    this->restoreGeo();
    this->startupWarning();

    this->initSkins();
    this->initStatusBar();
    this->initWidgets();
    this->initMenu();
    this->initTray();
    this->initHome();
    this->initTouchBar();
    this->initWalletContext();

    // Websocket notifier
    connect(websocketNotifier(), &WebsocketNotifier::CCSReceived, ui->ccsWidget->model(), &CCSModel::updateEntries);
    connect(websocketNotifier(), &WebsocketNotifier::RedditReceived, ui->redditWidget->model(), &RedditModel::updatePosts);
    connect(websocketNotifier(), &WebsocketNotifier::UpdatesReceived, this, &MainWindow::onUpdatesAvailable);
#ifdef HAS_XMRIG
    connect(websocketNotifier(), &WebsocketNotifier::XMRigDownloadsReceived, m_xmrig, &XMRigWidget::onDownloads);
#endif

    // Settings
    for (auto tickerWidget: m_tickerWidgets)
        connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, tickerWidget, &TickerWidget::init);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, m_balanceWidget, &TickerWidget::init);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, m_ctx, &AppContext::onPreferredFiatCurrencyChanged);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, ui->sendWidget, QOverload<>::of(&SendWidget::onPreferredFiatCurrencyChanged));
    connect(m_windowSettings, &Settings::amountPrecisionChanged, m_ctx, &AppContext::onAmountPrecisionChanged);
    connect(m_windowSettings, &Settings::skinChanged, this, &MainWindow::skinChanged);

    // Wizard
    connect(this, &MainWindow::closed, [=]{
        if (m_wizard)
            m_wizard->close();
    });

    // History
    // TODO: move this
    connect(m_ctx, &AppContext::walletRefreshed, ui->historyWidget, &HistoryWidget::onWalletRefreshed);
    connect(m_ctx, &AppContext::walletOpened, ui->historyWidget, &HistoryWidget::onWalletOpened);

    if (!config()->get(Config::firstRun).toBool() || TailsOS::detect() || WhonixOS::detect()) {
        this->onInitialNetworkConfigured();
    }

    this->setEnabled(true);
    this->show();
    ColorScheme::updateFromWidget(this);

    if (!this->autoOpenWallet()) {
        this->initWizard();
    }

    // Timers
    connect(&m_updateBytes, &QTimer::timeout, this, &MainWindow::updateNetStats);
    connect(&m_txTimer, &QTimer::timeout, [this]{
        m_statusLabelStatus->setText("Constructing transaction" + this->statusDots());
    });
}

void MainWindow::initSkins() {
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
    ColorScheme::updateFromWidget(this);
}

void MainWindow::initStatusBar() {
#if defined(Q_OS_WIN)
    // No seperators between statusbar widgets
    this->statusBar()->setStyleSheet("QStatusBar::item {border: None;}");
#endif

    this->statusBar()->setFixedHeight(30);

    m_statusLabelStatus = new QLabel("Idle", this);
    m_statusLabelStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addWidget(m_statusLabelStatus);

    m_statusLabelNetStats = new QLabel("", this);
    m_statusLabelNetStats->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addWidget(m_statusLabelNetStats);

    m_statusUpdateAvailable = new QPushButton(this);
    m_statusUpdateAvailable->setFlat(true);
    m_statusUpdateAvailable->setCursor(Qt::PointingHandCursor);
    m_statusUpdateAvailable->setIcon(icons()->icon("tab_party.png"));
    m_statusUpdateAvailable->hide();
    this->statusBar()->addPermanentWidget(m_statusUpdateAvailable);

    m_statusLabelBalance = new ClickableLabel(this);
    m_statusLabelBalance->setText("Balance: 0 XMR");
    m_statusLabelBalance->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_statusLabelBalance->setCursor(Qt::PointingHandCursor);
    this->statusBar()->addPermanentWidget(m_statusLabelBalance);
    connect(m_statusLabelBalance, &ClickableLabel::clicked, this, &MainWindow::showBalanceDialog);

    m_statusBtnConnectionStatusIndicator = new StatusBarButton(icons()->icon("status_disconnected.svg"), "Connection status", this);
    connect(m_statusBtnConnectionStatusIndicator, &StatusBarButton::clicked, this, &MainWindow::showConnectionStatusDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnConnectionStatusIndicator);

    m_statusBtnPassword = new StatusBarButton(icons()->icon("lock.svg"), "Password", this);
    connect(m_statusBtnPassword, &StatusBarButton::clicked, this, &MainWindow::showPasswordDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnPassword);

    m_statusBtnPreferences = new StatusBarButton(icons()->icon("preferences.svg"), "Settings", this);
    connect(m_statusBtnPreferences, &StatusBarButton::clicked, this, &MainWindow::menuSettingsClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnPreferences);

    m_statusBtnSeed = new StatusBarButton(icons()->icon("seed.png"), "Seed", this);
    connect(m_statusBtnSeed, &StatusBarButton::clicked, this, &MainWindow::showSeedDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnSeed);

    m_statusBtnTor = new StatusBarButton(icons()->icon("tor_logo_disabled.png"), "Tor", this);
    connect(m_statusBtnTor, &StatusBarButton::clicked, this, &MainWindow::menuTorClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnTor);

    m_statusBtnHwDevice = new StatusBarButton(icons()->icon("ledger.png"), "Ledger", this);
    connect(m_statusBtnHwDevice, &StatusBarButton::clicked, this, &MainWindow::menuHwDeviceClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnHwDevice);
    m_statusBtnHwDevice->hide();
}

void MainWindow::initWidgets() {
    int homeWidget = config()->get(Config::homeWidget).toInt();
    ui->tabHomeWidget->setCurrentIndex(TabsHome(homeWidget));
    connect(ui->tabHomeWidget, &QTabWidget::currentChanged, [](int index){
        config()->set(Config::homeWidget, TabsHome(index));
    });

    // [History]
    connect(ui->historyWidget, &HistoryWidget::viewOnBlockExplorer, this, &MainWindow::onViewOnBlockExplorer);
    connect(ui->historyWidget, &HistoryWidget::resendTransaction, this, &MainWindow::onResendTransaction);

    // [Receive]
    connect(ui->receiveWidget, &ReceiveWidget::showTransactions, [this](const QString &text) {
        ui->historyWidget->setSearchText(text);
        ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
    });
    connect(ui->contactWidget, &ContactsWidget::fillAddress, ui->sendWidget, &SendWidget::fillAddress);


#ifdef HAS_LOCALMONERO
    m_localMoneroWidget = new LocalMoneroWidget(this, m_ctx);
    ui->localMoneroLayout->addWidget(m_localMoneroWidget);
#else
    ui->tabWidgetExchanges->setTabVisible(0, false);
#endif

#ifdef HAS_XMRIG
    m_xmrig = new XMRigWidget(m_ctx, this);
    ui->xmrRigLayout->addWidget(m_xmrig);

    connect(m_xmrig, &XMRigWidget::miningStarted, [this]{ this->setTitle(true); });
    connect(m_xmrig, &XMRigWidget::miningEnded, [this]{ this->setTitle(false); });
#else
    ui->tabWidget->setTabVisible(Tabs::XMRIG, false);
#endif
}

void MainWindow::initMenu() {
    // TODO: Rename actions to follow style
    // [File]
    connect(ui->actionClose,    &QAction::triggered, this, &MainWindow::menuWalletCloseClicked); // Close current wallet
    connect(ui->actionQuit,     &QAction::triggered, this, &MainWindow::menuQuitClicked);        // Quit application
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::menuSettingsClicked);

    // [Wallet]
    connect(ui->actionInformation,  &QAction::triggered, this, &MainWindow::showWalletInfoDialog);
    connect(ui->actionPassword,     &QAction::triggered, this, &MainWindow::showPasswordDialog);
    connect(ui->actionSeed,         &QAction::triggered, this, &MainWindow::showSeedDialog);
    connect(ui->actionKeys,         &QAction::triggered, this, &MainWindow::showKeysDialog);
    connect(ui->actionViewOnly,     &QAction::triggered, this, &MainWindow::showViewOnlyDialog);

    // [Wallet] -> [Advanced]
    connect(ui->actionStore_wallet,          &QAction::triggered, [this]{m_ctx->currentWallet->store();});
    connect(ui->actionUpdate_balance,        &QAction::triggered, [this]{m_ctx->updateBalance();});
    connect(ui->actionRefresh_tabs,          &QAction::triggered, [this]{m_ctx->refreshModels();});
    connect(ui->actionRescan_spent,          &QAction::triggered, this, &MainWindow::rescanSpent);
    connect(ui->actionWallet_cache_debug,    &QAction::triggered, this, &MainWindow::showWalletCacheDebugDialog);
    connect(ui->actionChange_restore_height, &QAction::triggered, this, &MainWindow::showRestoreHeightDialog);

    // [Wallet] -> [Advanced] -> [Export]
    connect(ui->actionExportOutputs,   &QAction::triggered, this, &MainWindow::exportOutputs);
    connect(ui->actionExportKeyImages, &QAction::triggered, this, &MainWindow::exportKeyImages);

    // [Wallet] -> [Advanced] -> [Import]
    connect(ui->actionImportOutputs,   &QAction::triggered, this, &MainWindow::importOutputs);
    connect(ui->actionImportKeyImages, &QAction::triggered, this, &MainWindow::importKeyImages);

    // [Wallet] -> [History]
    connect(ui->actionExport_CSV, &QAction::triggered, this, &MainWindow::onExportHistoryCSV);

    // [Wallet] -> [Contacts]
    connect(ui->actionExportContactsCSV, &QAction::triggered, this, &MainWindow::onExportContactsCSV);
    connect(ui->actionImportContactsCSV, &QAction::triggered, this, &MainWindow::importContacts);

    // [View]
    m_tabShowHideSignalMapper = new QSignalMapper(this);

    // Show/Hide Home
    connect(ui->actionShow_Home, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Home"] = new ToggleTab(ui->tabHome, "Home", "Home", ui->actionShow_Home, Config::showTabHome);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Home, "Home");

    // Show/Hide Coins
    connect(ui->actionShow_Coins, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Coins"] = new ToggleTab(ui->tabCoins, "Coins", "Coins", ui->actionShow_Coins, Config::showTabCoins);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Coins, "Coins");

    // Show/Hide Calc
    connect(ui->actionShow_calc, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Calc"] = new ToggleTab(ui->tabCalc, "Calc", "Calc", ui->actionShow_calc, Config::showTabCalc);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_calc, "Calc");

    // Show/Hide Exchange
#if defined(HAS_LOCALMONERO)
    connect(ui->actionShow_Exchange, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Exchange"] = new ToggleTab(ui->tabExchange, "Exchange", "Exchange", ui->actionShow_Exchange, Config::showTabExchange);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Exchange, "Exchange");
#else
    ui->actionShow_Exchange->setVisible(false);
    ui->tabWidget->setTabVisible(Tabs::EXCHANGES, false);
#endif

    // Show/Hide Mining
#if defined(HAS_XMRIG)
    connect(ui->actionShow_XMRig, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Mining"] = new ToggleTab(ui->tabXmrRig, "Mining", "Mining", ui->actionShow_XMRig, Config::showTabXMRig);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_XMRig, "Mining");
#else
    ui->actionShow_XMRig->setVisible(false);
#endif

    for (const auto &key: m_tabShowHideMapper.keys()) {
        const auto toggleTab = m_tabShowHideMapper.value(key);
        const bool show = config()->get(toggleTab->configKey).toBool();
        toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    }
    connect(m_tabShowHideSignalMapper, &QSignalMapper::mappedString, this, &MainWindow::menuToggleTabVisible);

    // [Tools]
    connect(ui->actionSignVerify,                  &QAction::triggered, this, &MainWindow::menuSignVerifyClicked);
    connect(ui->actionVerifyTxProof,               &QAction::triggered, this, &MainWindow::menuVerifyTxProof);
    connect(ui->actionLoadUnsignedTxFromFile,      &QAction::triggered, this, &MainWindow::loadUnsignedTx);
    connect(ui->actionLoadUnsignedTxFromClipboard, &QAction::triggered, this, &MainWindow::loadUnsignedTxFromClipboard);
    connect(ui->actionLoadSignedTxFromFile,        &QAction::triggered, this, &MainWindow::loadSignedTx);
    connect(ui->actionLoadSignedTxFromText,        &QAction::triggered, this, &MainWindow::loadSignedTxFromText);
    connect(ui->actionImport_transaction,          &QAction::triggered, this, &MainWindow::importTransaction);
    connect(ui->actionPay_to_many,                 &QAction::triggered, this, &MainWindow::payToMany);
    connect(ui->actionCalculator,                  &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(ui->actionCreateDesktopEntry,          &QAction::triggered, this, &MainWindow::onCreateDesktopEntry);

    // TODO: Allow creating desktop entry on Windows and Mac
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    ui->actionCreateDesktopEntry->setDisabled(true);
#endif

    // [Help]
    connect(ui->actionAbout,             &QAction::triggered, this, &MainWindow::menuAboutClicked);
    connect(ui->actionOfficialWebsite,   &QAction::triggered, [this](){Utils::externalLinkWarning(this, "https://featherwallet.org");});
    connect(ui->actionDonate_to_Feather, &QAction::triggered, this, &MainWindow::donateButtonClicked);
    connect(ui->actionReport_bug,        &QAction::triggered, this, &MainWindow::onReportBug);
    connect(ui->actionShow_debug_info,   &QAction::triggered, this, &MainWindow::showDebugInfo);


    // Setup shortcuts
    ui->actionStore_wallet->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionRefresh_tabs->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionClose->setShortcut(QKeySequence("Ctrl+W"));
    ui->actionShow_debug_info->setShortcut(QKeySequence("Ctrl+D"));
    ui->actionSettings->setShortcut(QKeySequence("Ctrl+Alt+S"));
    ui->actionUpdate_balance->setShortcut(QKeySequence("Ctrl+U"));
}

void MainWindow::initTray() {
    // TODO: Add tray support on Windows and Mac

#if defined(Q_OS_LINUX)
    // system tray
    m_trayIcon = new QSystemTrayIcon(QIcon(":/assets/images/appicons/64x64.png"));
    m_trayIcon->show();

    m_trayActionCalc = new QAction("Calc", this);
    m_trayActionCalc->setStatusTip("Calculator");

    m_trayActionSend = new QAction("Send", this);
    m_trayActionSend->setStatusTip("Send XMR payment");

    m_trayActionHistory = new QAction("History", this);
    m_trayActionHistory->setStatusTip("View incoming transfers");

    m_trayActionExit = new QAction("Quit", this);
    m_trayActionExit->setStatusTip("Exit application");

    m_trayMenu.addAction(m_trayActionSend);
    m_trayMenu.addAction(m_trayActionHistory);
    m_trayMenu.addAction(m_trayActionCalc);
    m_trayMenu.addAction(m_trayActionExit);
    m_trayIcon->setContextMenu(&m_trayMenu);

    connect(m_trayActionCalc,    &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(m_trayActionSend,    &QAction::triggered, this, &MainWindow::showSendTab);
    connect(m_trayActionHistory, &QAction::triggered, this, &MainWindow::showHistoryTab);
    connect(m_trayActionExit,    &QAction::triggered, this, &QMainWindow::close);
#endif
}

void MainWindow::initHome() {
    // Ticker widgets
    m_tickerWidgets.append(new TickerWidget(this, m_ctx, "XMR"));
    m_tickerWidgets.append(new TickerWidget(this, m_ctx, "BTC"));
    for (auto tickerWidget: m_tickerWidgets) {
        ui->tickerLayout->addWidget(tickerWidget);
    }
    m_balanceWidget = new TickerWidget(this, m_ctx, "XMR", "Balance", true, true);
    ui->fiatTickerLayout->addWidget(m_balanceWidget);

    connect(ui->ccsWidget, &CCSWidget::selected, this, &MainWindow::showSendScreen);
    connect(ui->redditWidget, &RedditWidget::setStatusText, this, &MainWindow::setStatusText);
}

void MainWindow::initTouchBar() {
#ifdef Q_OS_MAC
    m_touchbar = new KDMacTouchBar(this);
    m_touchbarActionWelcome = new QAction(QIcon(":/assets/images/feather.png"), "Welcome to Feather!");
    m_touchbarWalletItems = {ui->actionSettings, ui->actionCalculator, ui->actionKeys, ui->actionDonate_to_Feather};
    m_touchbarWizardItems = {m_touchbarActionWelcome};
#endif
}

void MainWindow::initWalletContext() {
    connect(m_ctx, &AppContext::walletClosed,            [this](){this->onWalletClosed();});
    connect(m_ctx, &AppContext::balanceUpdated,           this, &MainWindow::onBalanceUpdated);
    connect(m_ctx, &AppContext::walletOpened,             this, &MainWindow::onWalletOpened);
    connect(m_ctx, &AppContext::walletOpenedError,        this, &MainWindow::onWalletOpenedError);
    connect(m_ctx, &AppContext::walletCreatedError,       this, &MainWindow::onWalletCreatedError);
    connect(m_ctx, &AppContext::synchronized,             this, &MainWindow::onSynchronized);
    connect(m_ctx, &AppContext::blockchainSync,           this, &MainWindow::onBlockchainSync);
    connect(m_ctx, &AppContext::refreshSync,              this, &MainWindow::onRefreshSync);
    connect(m_ctx, &AppContext::createTransactionError,   this, &MainWindow::onCreateTransactionError);
    connect(m_ctx, &AppContext::createTransactionSuccess, this, &MainWindow::onCreateTransactionSuccess);
    connect(m_ctx, &AppContext::transactionCommitted,     this, &MainWindow::onTransactionCommitted);
    connect(m_ctx, &AppContext::walletOpenPasswordNeeded, this, &MainWindow::onWalletOpenPasswordRequired);
    connect(m_ctx, &AppContext::deviceButtonRequest,      this, &MainWindow::onDeviceButtonRequest);
    connect(m_ctx, &AppContext::deviceError,              this, &MainWindow::onDeviceError);
    connect(m_ctx, &AppContext::donationNag,              this, &MainWindow::onShowDonationNag);
    connect(m_ctx, &AppContext::initiateTransaction,      this, &MainWindow::onInitiateTransaction);
    connect(m_ctx, &AppContext::endTransaction,           this, &MainWindow::onEndTransaction);
    connect(m_ctx, &AppContext::customRestoreHeightSet,   this, &MainWindow::onCustomRestoreHeightSet);
    connect(m_ctx, &AppContext::walletAboutToClose,       this, &MainWindow::onWalletAboutToClose);

    // Nodes
    connect(m_ctx->nodes, &Nodes::updateStatus, [=](const QString &msg){this->setStatusText(msg);});
    connect(m_ctx->nodes, &Nodes::nodeExhausted,   this, &MainWindow::showNodeExhaustedMessage);
    connect(m_ctx->nodes, &Nodes::WSNodeExhausted, this, &MainWindow::showWSNodeExhaustedMessage);
}

void MainWindow::initWizard() {
    this->setEnabled(false);
    auto startPage = WalletWizard::Page_Menu;
    if (config()->get(Config::firstRun).toBool() && !(TailsOS::detect() || WhonixOS::detect())) {
        startPage = WalletWizard::Page_Network;
    }

    m_wizard = this->createWizard(startPage);
    m_wizard->show();
    m_wizard->setEnabled(true);
    this->touchbarShowWizard();
}

void MainWindow::startupWarning() {
    // Stagenet / Testnet
    auto worthlessWarning = QString("Feather wallet is currently running in %1 mode. This is meant "
                                    "for developers only. Your coins are WORTHLESS.");
    if (m_ctx->networkType == NetworkType::STAGENET && config()->get(Config::warnOnStagenet).toBool()) {
        QMessageBox::warning(this, "Warning", worthlessWarning.arg("stagenet"));
        config()->set(Config::warnOnStagenet, false);
    }
    else if (m_ctx->networkType == NetworkType::TESTNET && config()->get(Config::warnOnTestnet).toBool()){
        QMessageBox::warning(this, "Warning", worthlessWarning.arg("testnet"));
        config()->set(Config::warnOnTestnet, false);
    }

    // Beta
    if (config()->get(Config::warnOnAlpha).toBool()) {
        QString warning = "Feather Wallet is currently in beta.\n\nPlease report any bugs "
                          "you encounter on our Git repository, IRC or on /r/FeatherWallet.";
        QMessageBox::warning(this, "Beta Warning", warning);
        config()->set(Config::warnOnAlpha, false);
    }
}

bool MainWindow::autoOpenWallet() {
    QString autoPath = config()->get(Config::autoOpenWalletPath).toString();
    if (!autoPath.isEmpty() && autoPath.startsWith(QString::number(m_ctx->networkType))) {
        autoPath.remove(0, 1);
    }
    if (!autoPath.isEmpty() && Utils::fileExists(autoPath)) {
        m_ctx->onOpenWallet(autoPath, m_ctx->walletPassword);
        return true;
    }
    return false;
}

void MainWindow::menuToggleTabVisible(const QString &key){
    const auto toggleTab = m_tabShowHideMapper[key];
    bool show = config()->get(toggleTab->configKey).toBool();
    show = !show;
    config()->set(toggleTab->configKey, show);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
}

WalletWizard *MainWindow::createWizard(WalletWizard::Page startPage){
    auto *wizard = new WalletWizard(m_ctx, startPage, this);
    connect(wizard, &WalletWizard::initialNetworkConfigured, this, &MainWindow::onInitialNetworkConfigured);
    connect(wizard, &WalletWizard::skinChanged, this, &MainWindow::skinChanged);
    connect(wizard, &WalletWizard::openWallet, m_ctx, &AppContext::onOpenWallet);
    connect(wizard, &WalletWizard::defaultWalletDirChanged, m_windowSettings, &Settings::updatePaths);
    connect(wizard, &WalletWizard::rejected, [this]{
        this->cleanupBeforeClose();
        QCoreApplication::quit();
    });
    return wizard;
}

void MainWindow::showWizard(WalletWizard::Page startPage) {
    this->setEnabled(false);
    if (m_wizard == nullptr)
        m_wizard = this->createWizard(startPage);
    m_wizard->setStartId(startPage);
    m_wizard->restart();
    m_wizard->setEnabled(true);
    m_wizard->show();
}

void MainWindow::onWalletClosed(WalletWizard::Page page) {
    m_statusLabelBalance->clear();
    m_statusLabelStatus->clear();
    this->showWizard(page);
}

void MainWindow::touchbarShowWizard() {
#ifdef Q_OS_MAC
    m_touchbar->clear();
    for(auto* action: m_touchbarWizardItems) m_touchbar->addAction(action);
#endif
}

void MainWindow::touchbarShowWallet() {
#ifdef Q_OS_MAC
    m_touchbar->clear();
    for(auto* action: m_touchbarWalletItems) m_touchbar->addAction(action);
#endif
}

void MainWindow::onWalletCreatedError(const QString &err) {
    this->displayWalletErrorMsg(err);
    m_splashDialog->hide();
    this->showWizard(WalletWizard::Page_Menu);
}

void MainWindow::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    QFileInfo fileInfo(path);

    auto dialog = new PasswordDialog(this, fileInfo.fileName(), invalidPassword);
    switch (dialog->exec()) {
        case QDialog::Rejected:
        {
            this->showWizard(WalletWizard::Page_OpenWallet);
            return;
        }
    }

    m_ctx->walletPassword = dialog->password;
    m_ctx->onOpenWallet(m_ctx->walletPath, m_ctx->walletPassword);

    dialog->deleteLater();
}

void MainWindow::onDeviceButtonRequest(quint64 code) {
    if (m_wizard) {
        m_wizard->hide();
    }

    m_splashDialog->setMessage("Action required on device: Export the view key to open the wallet.");
    m_splashDialog->setIcon(QPixmap(":/assets/images/key.png"));
    m_splashDialog->show();
    m_splashDialog->setEnabled(true);
}

void MainWindow::onWalletOpenedError(const QString &err) {
    qDebug() << Q_FUNC_INFO << QString("Wallet open error: %1").arg(err);
    m_splashDialog->hide();
    this->displayWalletErrorMsg(err);
    this->setWindowTitle("Feather");
    this->showWizard(WalletWizard::Page_OpenWallet);
    this->touchbarShowWizard();
}

void MainWindow::displayWalletErrorMsg(const QString &err) {
    QString errMsg = err;
    if (err.contains("No device found")) {
        errMsg += "\n\nThis wallet is backed by a hardware device. Make sure the Monero app is opened on the device.\n"
                  "You may need to restart Feather before the device can get detected.";
    }
    if (errMsg.contains("Unable to open device")) {
        errMsg += "\n\nThe device might be in use by a different application.";
    }

    if (errMsg.contains("SW_CLIENT_NOT_SUPPORTED")) {
        errMsg += "\n\nIncompatible version: you may need to upgrade the Monero app on the Ledger device to the latest version.";
    }
    else if (errMsg.contains("Wrong Device Status")) {
        errMsg += "\n\nThe device may need to be unlocked.";
    }
    else if (errMsg.contains("Wrong Channel")) {
        errMsg += "\n\nRestart the hardware device and try again.";
    }

    QMessageBox::warning(this, "Wallet error", errMsg);
}

void MainWindow::onWalletOpened() {
    qDebug() << Q_FUNC_INFO;
    m_splashDialog->hide();

    if (m_wizard) {
        m_wizard->hide();
    }

    if (m_ctx->currentWallet->isHwBacked()) {
        m_statusBtnHwDevice->show();
    }

    this->bringToFront();
    this->setEnabled(true);
    if(!torManager()->torConnected)
        this->setStatusText("Wallet opened - Starting Tor (may take a while)");
    else
        this->setStatusText("Wallet opened - Searching for node");

    connect(m_ctx->currentWallet, &Wallet::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);

    // receive page
    m_ctx->currentWallet->subaddress()->refresh( m_ctx->currentWallet->currentSubaddressAccount());
    ui->receiveWidget->setModel( m_ctx->currentWallet->subaddressModel(),  m_ctx->currentWallet);
    if (m_ctx->currentWallet->subaddress()->count() == 1) {
        for (int i = 0; i < 10; i++) {
            m_ctx->currentWallet->subaddress()->addRow(m_ctx->currentWallet->currentSubaddressAccount(), "");
        }
    }

    // history page
    m_ctx->currentWallet->history()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->historyWidget->setModel(m_ctx->currentWallet->historyModel(), m_ctx->currentWallet);

    // contacts widget
    ui->contactWidget->setModel(m_ctx->currentWallet->addressBookModel());

    // coins page
    m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->coinsWidget->setModel(m_ctx->currentWallet->coinsModel(), m_ctx->currentWallet->coins());

    this->touchbarShowWallet();
    this->updatePasswordIcon();

    this->setTitle(false);

    m_updateBytes.start(250);
}

void MainWindow::onBalanceUpdated(quint64 balance, quint64 spendable) {
    qDebug() << Q_FUNC_INFO;
    bool hide = config()->get(Config::hideBalance).toBool();

    QString balance_str = WalletManager::displayAmount(spendable);
    balance_str.remove(QRegExp("0+$"));

    QString label_str = QString("Balance: %1 XMR").arg(balance_str);
    if (balance > spendable) {
        QString unconfirmed_str = WalletManager::displayAmount(spendable);
        unconfirmed_str.remove(QRegExp("0+$"));
        label_str += QString(" (+%1 XMR unconfirmed)").arg(Utils::balanceFormat(balance - spendable));
    }


    if (hide)
        label_str = "Balance: HIDDEN";

    m_statusLabelBalance->setToolTip("Click for details");
    m_statusLabelBalance->setText(label_str);
    m_balanceWidget->setHidden(hide);
}

void MainWindow::setStatusText(const QString &text, bool override, int timeout) {
    if (override) {
        m_statusOverrideActive = true;
        m_statusLabelStatus->setText(text);
        QTimer::singleShot(timeout, [this]{
            m_statusOverrideActive = false;
            this->setStatusText(m_statusText);
        });
        return;
    }

    m_statusText = text;

    if (!m_statusOverrideActive && !m_constructingTransaction) {
        m_statusLabelStatus->setText(text);
    }
}

void MainWindow::onSynchronized() {
    this->updateNetStats();
    this->setStatusText("Synchronized");
}

void MainWindow::onBlockchainSync(int height, int target) {
    QString blocks = (target >= height) ? QString::number(target - height) : "?";
    QString heightText = QString("Blockchain sync: %1 blocks remaining").arg(blocks);
    this->setStatusText(heightText);
}

void MainWindow::onRefreshSync(int height, int target) {
    QString blocks = (target >= height) ? QString::number(target - height) : "?";
    QString heightText = QString("Wallet sync: %1 blocks remaining").arg(blocks);
    this->setStatusText(heightText);
}

void MainWindow::onConnectionStatusChanged(int status)
{
    qDebug() << "Wallet connection status changed " << Utils::QtEnumToString(static_cast<Wallet::ConnectionStatus>(status));

    // Update connection info in status bar.

    QIcon icon;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            icon = icons()->icon("status_disconnected.svg");
            this->setStatusText("Disconnected");
            break;
        case Wallet::ConnectionStatus_Connecting:
            icon = icons()->icon("status_lagging.svg");
            this->setStatusText("Connecting to node");
            break;
        case Wallet::ConnectionStatus_WrongVersion:
            icon = icons()->icon("status_disconnected.svg");
            this->setStatusText("Incompatible node");
            break;
        case Wallet::ConnectionStatus_Synchronizing:
            icon = icons()->icon("status_waiting.svg");
            break;
        case Wallet::ConnectionStatus_Synchronized:
            icon = icons()->icon("status_connected.svg");
            break;
        default:
            icon = icons()->icon("status_disconnected.svg");
            break;
    }

    m_statusBtnConnectionStatusIndicator->setIcon(icon);
}

void MainWindow::onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address) {
    auto tx_status = tx->status();
    auto err = QString("Can't create transaction: ");

    if(tx_status != PendingTransaction::Status_Ok){
        auto tx_err = tx->errorString();
        qCritical() << tx_err;

        if (m_ctx->currentWallet->connectionStatus() == Wallet::ConnectionStatus_WrongVersion)
            err = QString("%1 Wrong daemon version: %2").arg(err).arg(tx_err);
        else
            err = QString("%1 %2").arg(err).arg(tx_err);

        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_ctx->currentWallet->disposeTransaction(tx);
    } else if (tx->txCount() == 0) {
        err = QString("%1 %2").arg(err).arg("No unmixable outputs to sweep.");
        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_ctx->currentWallet->disposeTransaction(tx);
    } else {
        const auto &description = m_ctx->tmpTxDescription;

        // Show advanced dialog on multi-destination transactions
        if (address.size() > 1) {
            auto *dialog_adv = new TxConfAdvDialog(m_ctx, description, this);
            dialog_adv->setTransaction(tx);
            dialog_adv->exec();
            dialog_adv->deleteLater();
            return;
        }

        auto *dialog = new TxConfDialog(m_ctx, tx, address[0], description, this);
        switch (dialog->exec()) {
            case QDialog::Rejected:
            {
                if (!dialog->showAdvanced)
                    m_ctx->onCancelTransaction(tx, address);
                break;
            }
            case QDialog::Accepted:
                m_ctx->commitTransaction(tx);
                break;
        }

        if (dialog->showAdvanced) {
            auto *dialog_adv = new TxConfAdvDialog(m_ctx, description, this);
            dialog_adv->setTransaction(tx);
            dialog_adv->exec();
            dialog_adv->deleteLater();
        }
        dialog->deleteLater();
    }
}

void MainWindow::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid) {
    if (status) { // success
        QString body = QString("Successfully sent %1 transaction(s).").arg(txid.count());
        QMessageBox::information(this, "Transactions sent", body);
        ui->sendWidget->clearFields();
    } else {
        auto err = tx->errorString();
        QString body = QString("Error committing transaction: %1").arg(err);
        QMessageBox::warning(this, "Transaction failed", body);
    }
}

void MainWindow::onCreateTransactionError(const QString &message) {
    auto msg = QString("Error while creating transaction: %1").arg(message);

    if (msg.contains("failed to get random outs")) {
        msg += "\n\nYour transaction has too many inputs. Try sending a lower amount.";
    }

    QMessageBox::warning(this, "Transaction failed", msg);
}

void MainWindow::showWalletInfoDialog() {
    auto *dialog = new WalletInfoDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showSeedDialog() {
    if (m_ctx->currentWallet->isHwBacked()) {
        QMessageBox::information(this, "Information", "Seed unavailable: Wallet keys are stored on hardware device.");
        return;
    }

    if (m_ctx->currentWallet->viewOnly()) {
        QMessageBox::information(this, "Information", "Wallet is view-only and has no seed.\n\nTo obtain wallet keys go to Wallet -> View-Only");
        return;
    }

    if (!m_ctx->currentWallet->isDeterministic()) {
        QMessageBox::information(this, "Information", "Wallet is non-deterministic and has no seed.\n\nTo obtain wallet keys go to Wallet -> Keys");
        return;
    }

    auto *dialog = new SeedDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showConnectionStatusDialog() {
    auto status = m_ctx->currentWallet->connectionStatus();
    bool synchronized = m_ctx->currentWallet->isSynchronized();

    QString statusMsg;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            statusMsg = "Wallet is disconnected from daemon.";
            break;
        case Wallet::ConnectionStatus_Connecting: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connecting to %1").arg(node.toAddress());
            break;
        }
        case Wallet::ConnectionStatus_WrongVersion:
            statusMsg = "Wallet is connected to incompatible daemon.";
            break;
        case Wallet::ConnectionStatus_Synchronizing:
        case Wallet::ConnectionStatus_Synchronized: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connected to %1 ").arg(node.toAddress());

            if (synchronized) {
                statusMsg += "and synchronized";
            } else {
                statusMsg += "and synchronizing";
            }
            break;
        }
        default:
            statusMsg = "Unknown connection status (this should never happen).";
    }

    statusMsg += QString("\n\nTx: %1, Rx: %2").arg(Utils::formatBytes(m_ctx->currentWallet->getBytesSent()),
                                                   Utils::formatBytes(m_ctx->currentWallet->getBytesReceived()));

    QMessageBox::information(this, "Connection Status", statusMsg);
}

void MainWindow::showPasswordDialog() {
    auto *pdialog = new PasswordChangeDialog(this, m_ctx->currentWallet);
    pdialog->exec();
    pdialog->deleteLater();
    this->updatePasswordIcon();
}

void MainWindow::updatePasswordIcon() {
    QIcon icon = m_ctx->currentWallet->getPassword().isEmpty() ? icons()->icon("unlock.svg") : icons()->icon("lock.svg");
    m_statusBtnPassword->setIcon(icon);
}

void MainWindow::showRestoreHeightDialog() {
    // settings custom restore height is only available for 25 word seeds
    auto seed = m_ctx->currentWallet->getCacheAttribute("feather.seed");
    if(!seed.isEmpty()) {
        const auto msg = "This wallet has a 14 word mnemonic seed which has the restore height embedded.";
        QMessageBox::warning(this, "Cannot set custom restore height", msg);
        return;
    }

    m_restoreDialog = new RestoreDialog(m_ctx, this);
    m_restoreDialog->show();

    connect(m_restoreDialog, &RestoreDialog::accepted, [=]{
        auto height = m_restoreDialog->getHeight();
        m_restoreDialog->disconnect();
        m_restoreDialog->deleteLater();
        m_ctx->onSetRestoreHeight(height);
    });

    connect(m_restoreDialog, &RestoreDialog::rejected, [=]{
        m_restoreDialog->disconnect();
        m_restoreDialog->deleteLater();
    });
}

void MainWindow::showKeysDialog() {
    auto *dialog = new KeysDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showViewOnlyDialog() {
    auto *dialog = new ViewOnlyDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::menuTorClicked() {
    auto *dialog = new TorInfoDialog(this, m_ctx);
    connect(dialog, &TorInfoDialog::torSettingsChanged, m_ctx, &AppContext::onTorSettingsChanged);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::menuHwDeviceClicked() {
    QMessageBox::information(this, "Hardware Device", QString("This wallet is backed by a %1 hardware device.").arg(this->getHardwareDevice()));
}

void MainWindow::menuNewRestoreClicked() {
    // TODO: implement later
}

void MainWindow::menuQuitClicked() {
    cleanupBeforeClose();
    QCoreApplication::quit();
}

void MainWindow::menuWalletCloseClicked() {
    if (m_ctx->currentWallet == nullptr)
        return;

    m_ctx->closeWallet(true, true);
}

void MainWindow::menuAboutClicked() {
    AboutDialog dialog{this};
    dialog.exec();
}

void MainWindow::menuSettingsClicked() {
    m_windowSettings->raise();
    m_windowSettings->show();
    m_windowSettings->activateWindow();
}

void MainWindow::menuSignVerifyClicked() {
    SignVerifyDialog dialog{m_ctx->currentWallet, this};
    dialog.exec();
}

void MainWindow::menuVerifyTxProof() {
    VerifyProofDialog dialog{m_ctx->currentWallet, this};
    dialog.exec();
}

void MainWindow::skinChanged(const QString &skinName) {
    if (!m_skins.contains(skinName)) {
        qWarning() << QString("No such skin %1").arg(skinName);
        return;
    }

    config()->set(Config::skin, skinName);
    qApp->setStyleSheet(m_skins[skinName]);
    qDebug() << QString("Skin changed to %1").arg(skinName);
    ColorScheme::updateFromWidget(this);

#ifdef HAS_LOCALMONERO
    m_localMoneroWidget->skinChanged();
#endif

    ui->conversionWidget->skinChanged();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    cleanupBeforeClose();

    QWidget::closeEvent(event);
}

void MainWindow::donateButtonClicked() {
    double donation = appData()->prices.convert("EUR", "XMR", globals::donationAmount);
    if (donation <= 0)
        donation = 0.1337;

    ui->sendWidget->fill(globals::donationAddress, "Donation to the Feather development team", donation);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::showHistoryTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
}

void MainWindow::showSendTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::showCalcWindow() {
    m_windowCalc->show();
}

void MainWindow::payToMany() {
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
    ui->sendWidget->payToMany();
    QMessageBox::information(this, "Pay to many", "Enter a list of outputs in the 'Pay to' field.\n"
                                                  "One output per line.\n"
                                                  "Format: address, amount\n"
                                                  "A maximum of 16 addresses may be specified.");
}

void MainWindow::showSendScreen(const CCSEntry &entry) {
    ui->sendWidget->fill(entry);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::onViewOnBlockExplorer(const QString &txid) {
    QString blockExplorerLink = Utils::blockExplorerLink(config()->get(Config::blockExplorer).toString(), m_ctx->networkType, txid);
    Utils::externalLinkWarning(this, blockExplorerLink);
}

void MainWindow::onResendTransaction(const QString &txid) {
    if (!AppContext::txCache.contains(txid)) {
        QMessageBox::warning(this, "Unable to resend transaction", "Transaction was not found in transaction cache. Unable to resend.");
        return;
    }

    // Connect to a different node so chances of successful relay are higher
    m_ctx->nodes->autoConnect(true);

    auto dialog = new BroadcastTxDialog(this, m_ctx, AppContext::txCache[txid]);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::importContacts() {
    const QString targetFile = QFileDialog::getOpenFileName(this, "Import CSV file", QDir::homePath(), "CSV Files (*.csv)");
    if(targetFile.isEmpty()) return;

    auto *model = m_ctx->currentWallet->addressBookModel();
    QMapIterator<QString, QString> i(model->readCSV(targetFile));
    int inserts = 0;
    while (i.hasNext()) {
        i.next();
        bool addressValid = WalletManager::addressValid(i.value(), m_ctx->currentWallet->nettype());
        if(addressValid) {
            m_ctx->currentWallet->addressBook()->addRow(i.value(), "", i.key());
            inserts++;
        }
    }

    QMessageBox::information(this, "Contacts imported", QString("Total contacts imported: %1").arg(inserts));
}

MainWindow *MainWindow::getInstance() {
    return pMainWindow;
}

AppContext *MainWindow::getContext(){
    return pMainWindow->m_ctx;
}

QString MainWindow::loadStylesheet(const QString &resource) {
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

void MainWindow::saveGeo() {
    config()->set(Config::geometry, QString(saveGeometry().toBase64()));
    config()->set(Config::windowState, QString(saveState().toBase64()));
}

void MainWindow::restoreGeo() {
    bool geo = this->restoreGeometry(QByteArray::fromBase64(config()->get(Config::geometry).toByteArray()));
    bool windowState = this->restoreState(QByteArray::fromBase64(config()->get(Config::windowState).toByteArray()));
    qDebug() << "Restored window state: " << geo << " " << windowState;
}

void MainWindow::showDebugInfo() {
    auto *dialog = new DebugInfoDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showWalletCacheDebugDialog() {
    auto *dialog = new WalletCacheDebugDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showNodeExhaustedMessage() {
    // Spawning dialogs inside a lambda can cause system freezes on linux so we have to do it this way ¯\_(ツ)_/¯

    auto msg = "Feather is in 'custom node connection mode' but could not "
               "find an eligible node to connect to. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

void MainWindow::showWSNodeExhaustedMessage() {
    auto msg = "Feather is in 'automatic node connection mode' but the "
               "websocket server returned no available nodes. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

void MainWindow::exportKeyImages() {
    QString fn = QFileDialog::getSaveFileName(this, "Save key images to file", QDir::homePath(), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_keyImages")) fn += "_keyImages";
    m_ctx->currentWallet->exportKeyImages(fn, true);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Key image export", QString("Failed to export key images.\nReason: %1").arg(err));
    } else {
        QMessageBox::information(this, "Key image export", "Successfully exported key images.");
    }
}

void MainWindow::importKeyImages() {
    QString fn = QFileDialog::getOpenFileName(this, "Import key image file", QDir::homePath(), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    m_ctx->currentWallet->importKeyImages(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Key image import", QString("Failed to import key images.\n\n%1").arg(err));
    } else {
        QMessageBox::information(this, "Key image import", "Successfully imported key images");
        m_ctx->refreshModels();
    }
}

void MainWindow::exportOutputs() {
    QString fn = QFileDialog::getSaveFileName(this, "Save outputs to file", QDir::homePath(), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_outputs")) fn += "_outputs";
    m_ctx->currentWallet->exportOutputs(fn, true);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Outputs export", QString("Failed to export outputs.\nReason: %1").arg(err));
    } else {
        QMessageBox::information(this, "Outputs export", "Successfully exported outputs.");
    }
}

void MainWindow::importOutputs() {
    QString fn = QFileDialog::getOpenFileName(this, "Import outputs file", QDir::homePath(), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    m_ctx->currentWallet->importOutputs(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Outputs import", QString("Failed to import outputs.\n\n%1").arg(err));
    } else {
        QMessageBox::information(this, "Outputs import", "Successfully imported outputs");
        m_ctx->refreshModels();
    }
}

void MainWindow::cleanupBeforeClose() {
    m_ctx->closeWallet(false, true);
    torManager()->stop();
    this->saveGeo();
}

void MainWindow::loadUnsignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*unsigned_monero_tx)");
    if (fn.isEmpty()) return;
    UnsignedTransaction *tx = m_ctx->currentWallet->loadTxFile(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load transaction from file", QString("Failed to load transaction.\n\n%1").arg(err));
        return;
    }

    this->createUnsignedTxDialog(tx);
}

void MainWindow::loadUnsignedTxFromClipboard() {
    QString unsigned_tx = Utils::copyFromClipboard();
    if (unsigned_tx.isEmpty()) {
        QMessageBox::warning(this, "Load unsigned transaction from clipboard", "Clipboard is empty");
        return;
    }
    UnsignedTransaction *tx = m_ctx->currentWallet->loadTxFromBase64Str(unsigned_tx);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load unsigned transaction from clipboard", QString("Failed to load transaction.\n\n%1").arg(err));
        return;
    }

    this->createUnsignedTxDialog(tx);
}

void MainWindow::loadSignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) return;
    PendingTransaction *tx = m_ctx->currentWallet->loadSignedTxFile(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load signed transaction from file", err);
        return;
    }

    auto *dialog = new TxConfAdvDialog(m_ctx, "", this);
    dialog->setTransaction(tx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::loadSignedTxFromText() {
    auto dialog = new BroadcastTxDialog(this, m_ctx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::createUnsignedTxDialog(UnsignedTransaction *tx) {
    auto *dialog = new TxConfAdvDialog(m_ctx, "", this);
    dialog->setUnsignedTransaction(tx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::importTransaction() {

    if (config()->get(Config::torPrivacyLevel).toInt() == Config::allTorExceptNode) {
        // TODO: don't show if connected to local node

        auto result = QMessageBox::warning(this, "Warning", "Using this feature may allow a remote node to associate the transaction with your IP address.\n"
                                                            "\n"
                                                            "Connect to a trusted node or run Feather over Tor if network level metadata leakage is included in your threat model.",
                                           QMessageBox::Ok | QMessageBox::Cancel);
        if (result != QMessageBox::Ok) {
            return;
        }
    }

    auto *dialog = new TxImportDialog(this, m_ctx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::onDeviceError(const QString &error) {
    if (m_showDeviceError) {
        return;
    }
    m_statusBtnHwDevice->setIcon(icons()->icon("ledger_unpaired.png"));
    while (true) {
        m_showDeviceError = true;
        auto result = QMessageBox::question(this, "Hardware device", "Lost connection to hardware device. Attempt to reconnect?");
        if (result == QMessageBox::Yes) {
            bool r = m_ctx->currentWallet->reconnectDevice();
            if (r) {
                break;
            }
        }
        if (result == QMessageBox::No){
            m_ctx->closeWallet(true);
            return;
        }
    }
    m_statusBtnHwDevice->setIcon(icons()->icon("ledger.png"));
    m_ctx->currentWallet->startRefresh();
    m_showDeviceError = false;
}

void MainWindow::updateNetStats() {
    if (m_ctx->currentWallet == nullptr) {
        m_statusLabelNetStats->setText("");
        return;
    }

    if (m_ctx->currentWallet->connectionStatus() == Wallet::ConnectionStatus_Disconnected) {
        m_statusLabelNetStats->setText("");
        return;
    }

    if (m_ctx->currentWallet->connectionStatus() == Wallet::ConnectionStatus_Synchronized) {
        m_statusLabelNetStats->setText("");
        return;
    }


    m_statusLabelNetStats->setText(QString("(D: %1)").arg(Utils::formatBytes(m_ctx->currentWallet->getBytesReceived())));
}

void MainWindow::rescanSpent() {
    if (!m_ctx->currentWallet->rescanSpent()) {
        QMessageBox::warning(this, "Rescan spent", m_ctx->currentWallet->errorString());
    } else {
        QMessageBox::information(this, "Rescan spent", "Successfully rescanned spent outputs.");
    }
}

void MainWindow::showBalanceDialog() {
    if (!m_ctx->currentWallet) {
        return;
    }
    auto *dialog = new BalanceDialog(this, m_ctx->currentWallet);
    dialog->exec();
    dialog->deleteLater();
}

QString MainWindow::statusDots() {
    m_statusDots++;
    m_statusDots = m_statusDots % 4;
    return QString(".").repeated(m_statusDots);
}

void MainWindow::bringToFront() {
    ensurePolished();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    raise();
    activateWindow();
}

void MainWindow::onInitialNetworkConfigured() {
    m_ctx->onInitialNetworkConfigured();

    connect(torManager(), &TorManager::connectionStateChanged, [this](bool connected){
        connected ? m_statusBtnTor->setIcon(icons()->icon("tor_logo.png"))
                  : m_statusBtnTor->setIcon(icons()->icon("tor_logo_disabled.png"));});
}

void MainWindow::onCheckUpdatesComplete(const QString &version, const QString &binaryFilename,
                                        const QString &hash, const QString &signer) {
    QString versionDisplay{version};
    versionDisplay.replace("beta", "Beta");
    QString updateText = QString("Update to Feather %1 is available").arg(versionDisplay);
    m_statusUpdateAvailable->setText(updateText);
    m_statusUpdateAvailable->setToolTip("Click to Download update.");
    m_statusUpdateAvailable->show();

    m_statusUpdateAvailable->disconnect();
    connect(m_statusUpdateAvailable, &StatusBarButton::clicked, [this, version, binaryFilename, hash, signer] {
        this->onShowUpdateCheck(version, binaryFilename, hash, signer);
    });
}

void MainWindow::onShowUpdateCheck(const QString &version, const QString &binaryFilename,
                                   const QString &hash, const QString &signer) {
    QString downloadUrl = QString("https://featherwallet.org/files/releases/%1/%2").arg(this->getPlatformTag(), binaryFilename);

    UpdateDialog updateDialog{this, version, downloadUrl, hash, signer};
    connect(&updateDialog, &UpdateDialog::restartWallet, this, &MainWindow::onRestartApplication);
    updateDialog.exec();
}

void MainWindow::onUpdatesAvailable(const QJsonObject &updates) {
    QString featherVersionStr{FEATHER_VERSION};

    auto featherVersion = SemanticVersion::fromString(featherVersionStr);

    QString platformTag = getPlatformTag();
    if (platformTag.isEmpty()) {
        qWarning() << "Unsupported platform, unable to fetch update";
        return;
    }

    QJsonObject platformData = updates["platform"].toObject()[platformTag].toObject();
    if (platformData.isEmpty()) {
        qWarning() << "Unable to find current platform in updates data";
        return;
    }

    QString newVersion = platformData["version"].toString();
    if (SemanticVersion::fromString(newVersion) <= featherVersion) {
        return;
    }

    // Hooray! New update available

    QString hashesUrl = QString("%1/files/releases/hashes-%2-plain.txt").arg(globals::websiteUrl, newVersion);

    UtilsNetworking network{getNetworkTor()};
    QNetworkReply *reply = network.get(hashesUrl);

    connect(reply, &QNetworkReply::finished, this, std::bind(&MainWindow::onSignedHashesReceived, this, reply, platformTag, newVersion));
}

void MainWindow::onSignedHashesReceived(QNetworkReply *reply, const QString &platformTag, const QString &version) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unable to fetch signed hashes: " << reply->errorString();
        return;
    }

    QByteArray armoredSignedHashes = reply->readAll();
    reply->deleteLater();

    const QString binaryFilename = QString("feather-%1-%2.zip").arg(version, platformTag);
    QString signer;
    QByteArray signedHash = AsyncTask::runAndWaitForFuture([armoredSignedHashes, binaryFilename, &signer]{
        try {
            return Updater().verifyParseSignedHashes(armoredSignedHashes, binaryFilename, signer);
        }
        catch (const std::exception &e) {
            qWarning() << "Failed to fetch and verify signed hash: " << e.what();
            return QByteArray{};
        }
    });
    if (signedHash.isEmpty()) {
        return;
    }

    QString hash = signedHash.toHex();
    qInfo() << "Update found: " << binaryFilename << hash << "signed by:" << signer;
    this->onCheckUpdatesComplete(version, binaryFilename, hash, signer);
}

void MainWindow::onShowDonationNag() {
    auto msg = "Feather is a 100% community-sponsored endeavor. Please consider supporting "
               "the project financially. Get rid of this message by donating any amount.";
    int ret = QMessageBox::information(this, "Donate to Feather", msg, QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        this->donateButtonClicked();
    }
}

void MainWindow::onInitiateTransaction() {
    m_statusDots = 0;
    m_constructingTransaction = true;
    m_txTimer.start(1000);

    if (m_ctx->currentWallet->isHwBacked()) {
        QString message = "Constructing transaction: action may be required on device.";
        m_splashDialog->setMessage(message);
        m_splashDialog->setIcon(QPixmap(":/assets/images/unconfirmed.png"));
        m_splashDialog->show();
        m_splashDialog->setEnabled(true);
    }
}

void MainWindow::onEndTransaction() {
    // Todo: endTransaction can fail to fire when the node is switched during tx creation
    m_constructingTransaction = false;
    m_txTimer.stop();
    this->setStatusText(m_statusText);

    if (m_ctx->currentWallet->isHwBacked()) {
        m_splashDialog->hide();
    }
}

void MainWindow::onCustomRestoreHeightSet(int height) {
    auto msg = QString("The restore height for this wallet has been set to %1. "
                       "Please re-open the wallet. Feather will now quit.").arg(height);
    QMessageBox::information(this, "Cannot set custom restore height", msg);
    this->menuQuitClicked();
}

void MainWindow::onWalletAboutToClose() {
    if (!config()->get(Config::showTabHome).toBool())
        ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
    else
        ui->tabWidget->setCurrentIndex(Tabs::HOME);

    // Clear all tables when wallet is closed
    ui->historyWidget->resetModel();
    ui->contactWidget->resetModel();
    ui->receiveWidget->resetModel();
    ui->coinsWidget->resetModel();
}

void MainWindow::onExportHistoryCSV(bool checked) {
    if (m_ctx->currentWallet == nullptr)
        return;
    QString fn = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath(), "CSV (*.csv)");
    if (fn.isEmpty())
        return;
    if (!fn.endsWith(".csv"))
        fn += ".csv";
    m_ctx->currentWallet->history()->writeCSV(fn);
    QMessageBox::information(this, "CSV export", QString("Transaction history exported to %1").arg(fn));
}

void MainWindow::onExportContactsCSV(bool checked) {
    if (m_ctx->currentWallet == nullptr) return;
    auto *model = m_ctx->currentWallet->addressBookModel();
    if (model->rowCount() <= 0){
        QMessageBox::warning(this, "Error", "Addressbook empty");
        return;
    }

    const QString targetDir = QFileDialog::getExistingDirectory(this, "Select CSV output directory ", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if(targetDir.isEmpty()) return;

    qint64 now = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    QString fn = QString("%1/monero-contacts_%2.csv").arg(targetDir, QString::number(now / 1000));
    if(model->writeCSV(fn))
        QMessageBox::information(this, "Address book exported", QString("Address book exported to %1").arg(fn));
}

void MainWindow::onCreateDesktopEntry(bool checked) {
    auto msg = Utils::xdgDesktopEntryRegister() ? "Desktop entry created" : "Desktop entry not created due to an error.";
    QMessageBox::information(this, "Desktop entry", msg);
}

void MainWindow::onReportBug(bool checked) {
    QMessageBox::information(this, "Reporting Bugs",
                             "<body>Please report any bugs as issues on our git repo:<br>\n"
                             "<a href=\"https://git.featherwallet.org/feather/feather/issues\" style=\"color: #33A4DF\">https://git.featherwallet.org/feather/feather/issues</a><br/><br/>"
                             "\n"
                             "Before reporting a bug, upgrade to the most recent version of Feather "
                             "(latest release or git HEAD), and include the version number in your report. "
                             "Try to explain not only what the bug is, but how it occurs.</body>");
}

void MainWindow::onRestartApplication(const QString &binaryFilename) {
    QProcess::startDetached(binaryFilename, qApp->arguments());

    this->cleanupBeforeClose();
    QCoreApplication::quit();
}

QString MainWindow::getPlatformTag() {
#ifdef Q_OS_MACOS
    return "mac";
#endif
#ifdef Q_OS_WIN
    return "win";
#endif
#ifdef Q_OS_LINUX
    if (!qgetenv("APPIMAGE").isEmpty()) {
        return "linux-appimage";
    }
    return "linux";
#endif
    return "";
}

QString MainWindow::getHardwareDevice() {
    if (!m_ctx->currentWallet->isHwBacked())
        return "";
    if (m_ctx->currentWallet->isTrezor())
        return "Trezor";
    if (m_ctx->currentWallet->isLedger())
        return "Ledger";
    return "Unknown";
}

void MainWindow::setTitle(bool mining) {
    QFileInfo fileInfo(m_ctx->walletPath);
    auto title = QString("Feather - [%1]").arg(fileInfo.fileName());
    if (m_ctx->currentWallet && m_ctx->currentWallet->viewOnly())
        title += " [view-only]";
    if (mining)
        title += " [mining]";

    this->setWindowTitle(title);
}

MainWindow::~MainWindow() {
    delete ui;
}
