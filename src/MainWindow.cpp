// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "config-feather.h"
#include "constants.h"
#include "dialog/BalanceDialog.h"
#include "dialog/DebugInfoDialog.h"
#include "dialog/PasswordDialog.h"
#include "dialog/TorInfoDialog.h"
#include "dialog/TxBroadcastDialog.h"
#include "dialog/TxConfAdvDialog.h"
#include "dialog/TxConfDialog.h"
#include "dialog/TxImportDialog.h"
#include "dialog/TxInfoDialog.h"
#include "dialog/ViewOnlyDialog.h"
#include "dialog/WalletInfoDialog.h"
#include "dialog/WalletCacheDebugDialog.h"
#include "dialog/UpdateDialog.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/CoinsInfo.h"
#include "libwalletqt/Transfer.h"
#include "utils/AppData.h"
#include "utils/AsyncTask.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/NetworkManager.h"
#include "utils/os/tails.h"
#include "utils/SemanticVersion.h"
#include "utils/TorManager.h"
#include "utils/Updater.h"
#include "utils/WebsocketNotifier.h"

//#include "misc_log_ex.h"

MainWindow::MainWindow(WindowManager *windowManager, Wallet *wallet, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_windowManager(windowManager)
    , m_wallet(wallet)
    , m_nodes(new Nodes(this, wallet))
    , m_rpc(new DaemonRpc(this, ""))
{
    ui->setupUi(this);

//    MCWARNING("feather", "Platform tag: " << this->getPlatformTag().toStdString());

    // Ensure the destructor is called after closeEvent()
    setAttribute(Qt::WA_DeleteOnClose);

    m_windowCalc = new CalcWindow(this);
    m_splashDialog = new SplashDialog(this);
    m_accountSwitcherDialog = new AccountSwitcherDialog(m_wallet, this);

    m_updater = QSharedPointer<Updater>(new Updater(this));

    this->restoreGeo();

    this->initStatusBar();
    this->initWidgets();
    this->initMenu();
    this->initHome();
    this->initWalletContext();

    // Websocket notifier
    connect(websocketNotifier(), &WebsocketNotifier::CCSReceived, ui->ccsWidget->model(), &CCSModel::updateEntries);
    connect(websocketNotifier(), &WebsocketNotifier::BountyReceived, ui->bountiesWidget->model(), &BountiesModel::updateBounties);
    connect(websocketNotifier(), &WebsocketNotifier::RedditReceived, ui->redditWidget->model(), &RedditModel::updatePosts);
    connect(websocketNotifier(), &WebsocketNotifier::RevuoReceived, ui->revuoWidget, &RevuoWidget::updateItems);
    connect(websocketNotifier(), &WebsocketNotifier::UpdatesReceived, m_updater.data(), &Updater::wsUpdatesReceived);
#ifdef HAS_XMRIG
    connect(websocketNotifier(), &WebsocketNotifier::XMRigDownloadsReceived, m_xmrig, &XMRigWidget::onDownloads);
#endif
    websocketNotifier()->emitCache(); // Get cached data

    connect(m_windowManager, &WindowManager::websocketStatusChanged, this, &MainWindow::onWebsocketStatusChanged);
    this->onWebsocketStatusChanged(!config()->get(Config::disableWebsocket).toBool());

    connect(m_windowManager, &WindowManager::proxySettingsChanged, this, &MainWindow::onProxySettingsChanged);
    connect(m_windowManager, &WindowManager::updateBalance, m_wallet, &Wallet::updateBalance);
    connect(m_windowManager, &WindowManager::offlineMode, this, &MainWindow::onOfflineMode);

    connect(torManager(), &TorManager::connectionStateChanged, this, &MainWindow::onTorConnectionStateChanged);
    this->onTorConnectionStateChanged(torManager()->torConnected);

    connect(m_updater.data(), &Updater::updateAvailable, this, &MainWindow::showUpdateNotification);

    ColorScheme::updateFromWidget(this);
    QTimer::singleShot(1, [this]{this->updateWidgetIcons();});

    // Timers
    connect(&m_updateBytes, &QTimer::timeout, this, &MainWindow::updateNetStats);
    connect(&m_txTimer, &QTimer::timeout, [this]{
        m_statusLabelStatus->setText("Constructing transaction" + this->statusDots());
    });

    config()->set(Config::firstRun, false);

    this->onWalletOpened();

#ifdef DONATE_BEG
    this->donationNag();
#endif

    connect(m_windowManager->eventFilter, &EventFilter::userActivity, this, &MainWindow::userActivity);
    connect(&m_checkUserActivity, &QTimer::timeout, this, &MainWindow::checkUserActivity);
    m_checkUserActivity.setInterval(5000);
    m_checkUserActivity.start();
}

void MainWindow::initStatusBar() {
#if defined(Q_OS_WIN)
    // No separators between statusbar widgets
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
    connect(m_statusBtnConnectionStatusIndicator, &StatusBarButton::clicked, [this](){
        this->onShowSettingsPage(Settings::Pages::NETWORK);
    });
    this->statusBar()->addPermanentWidget(m_statusBtnConnectionStatusIndicator);
    this->onConnectionStatusChanged(Wallet::ConnectionStatus_Disconnected);

    m_statusAccountSwitcher = new StatusBarButton(icons()->icon("change_account.png"), "Account switcher", this);
    connect(m_statusAccountSwitcher, &StatusBarButton::clicked, this, &MainWindow::showAccountSwitcherDialog);
    this->statusBar()->addPermanentWidget(m_statusAccountSwitcher);

    m_statusBtnPassword = new StatusBarButton(icons()->icon("lock.svg"), "Password", this);
    connect(m_statusBtnPassword, &StatusBarButton::clicked, this, &MainWindow::showPasswordDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnPassword);

    m_statusBtnPreferences = new StatusBarButton(icons()->icon("preferences.svg"), "Settings", this);
    connect(m_statusBtnPreferences, &StatusBarButton::clicked, this, &MainWindow::menuSettingsClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnPreferences);

    m_statusBtnSeed = new StatusBarButton(icons()->icon("seed.png"), "Seed", this);
    connect(m_statusBtnSeed, &StatusBarButton::clicked, this, &MainWindow::showSeedDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnSeed);

    m_statusBtnProxySettings = new StatusBarButton(icons()->icon("tor_logo_disabled.png"), "Proxy settings", this);
    connect(m_statusBtnProxySettings, &StatusBarButton::clicked, this, &MainWindow::menuProxySettingsClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnProxySettings);
    this->onProxySettingsChanged();

    m_statusBtnHwDevice = new StatusBarButton(this->hardwareDevicePairedIcon(), this->getHardwareDevice(), this);
    connect(m_statusBtnHwDevice, &StatusBarButton::clicked, this, &MainWindow::menuHwDeviceClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnHwDevice);
    m_statusBtnHwDevice->hide();
}

void MainWindow::initWidgets() {
    int homeWidget = config()->get(Config::homeWidget).toInt();
    ui->tabHomeWidget->setCurrentIndex(TabsHome(homeWidget));

    // [History]
    m_historyWidget = new HistoryWidget(m_wallet, this);
    ui->historyWidgetLayout->addWidget(m_historyWidget);
    connect(m_historyWidget, &HistoryWidget::viewOnBlockExplorer, this, &MainWindow::onViewOnBlockExplorer);
    connect(m_historyWidget, &HistoryWidget::resendTransaction, this, &MainWindow::onResendTransaction);

    // [Send]
    m_sendWidget = new SendWidget(m_wallet, this);
    ui->sendWidgetLayout->addWidget(m_sendWidget);
    // --------------
    m_contactsWidget = new ContactsWidget(m_wallet, this);
    ui->contactsWidgetLayout->addWidget(m_contactsWidget);

    // [Receive]
    m_receiveWidget = new ReceiveWidget(m_wallet, this);
    ui->receiveWidgetLayout->addWidget(m_receiveWidget);
    connect(m_receiveWidget, &ReceiveWidget::showTransactions, [this](const QString &text) {
        m_historyWidget->setSearchText(text);
        ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
    });
    connect(m_contactsWidget, &ContactsWidget::fillAddress, m_sendWidget, &SendWidget::fillAddress);

    // [Coins]
    m_coinsWidget = new CoinsWidget(m_wallet, this);
    ui->coinsWidgetLayout->addWidget(m_coinsWidget);

#ifdef HAS_LOCALMONERO
    m_localMoneroWidget = new LocalMoneroWidget(this, m_wallet);
    ui->localMoneroLayout->addWidget(m_localMoneroWidget);
#else
    ui->tabWidgetExchanges->setTabVisible(0, false);
#endif

#ifdef HAS_XMRIG
    m_xmrig = new XMRigWidget(m_wallet, this);
    ui->xmrRigLayout->addWidget(m_xmrig);

    connect(m_xmrig, &XMRigWidget::miningStarted, [this]{ this->updateTitle(); });
    connect(m_xmrig, &XMRigWidget::miningEnded, [this]{ this->updateTitle(); });
#else
    ui->tabWidget->setTabVisible(Tabs::XMRIG, false);
#endif

#if defined(Q_OS_MACOS)
    ui->line->hide();
#endif

    ui->frame_coinControl->setVisible(false);
    connect(ui->btn_resetCoinControl, &QPushButton::clicked, [this]{
       m_wallet->setSelectedInputs({});
    });

    m_walletUnlockWidget = new WalletUnlockWidget(this);
    m_walletUnlockWidget->setWalletName(this->walletName());
    ui->walletUnlockLayout->addWidget(m_walletUnlockWidget);

    connect(m_walletUnlockWidget, &WalletUnlockWidget::closeWallet, this, &MainWindow::close);
    connect(m_walletUnlockWidget, &WalletUnlockWidget::unlockWallet, this, &MainWindow::unlockWallet);

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::initMenu() {
    // TODO: Rename actions to follow style
    // [File]
    connect(ui->actionOpen,        &QAction::triggered, this, &MainWindow::menuOpenClicked);
    connect(ui->actionNew_Restore, &QAction::triggered, this, &MainWindow::menuNewRestoreClicked);
    connect(ui->actionLock,        &QAction::triggered, this, &MainWindow::lockWallet);
    connect(ui->actionClose,       &QAction::triggered, this, &MainWindow::menuWalletCloseClicked); // Close current wallet
    connect(ui->actionQuit,        &QAction::triggered, this, &MainWindow::menuQuitClicked);        // Quit application
    connect(ui->actionSettings,    &QAction::triggered, this, &MainWindow::menuSettingsClicked);

    // [File] -> [Recently open]
    m_clearRecentlyOpenAction = new QAction("Clear history", ui->menuFile);
    connect(m_clearRecentlyOpenAction, &QAction::triggered, this, &MainWindow::menuClearHistoryClicked);

    // [Wallet]
    connect(ui->actionInformation,  &QAction::triggered, this, &MainWindow::showWalletInfoDialog);
    connect(ui->actionAccount,      &QAction::triggered, this, &MainWindow::showAccountSwitcherDialog);
    connect(ui->actionPassword,     &QAction::triggered, this, &MainWindow::showPasswordDialog);
    connect(ui->actionSeed,         &QAction::triggered, this, &MainWindow::showSeedDialog);
    connect(ui->actionKeys,         &QAction::triggered, this, &MainWindow::showKeysDialog);
    connect(ui->actionViewOnly,     &QAction::triggered, this, &MainWindow::showViewOnlyDialog);

    // [Wallet] -> [Advanced]
    connect(ui->actionStore_wallet,          &QAction::triggered, this, &MainWindow::tryStoreWallet);
    connect(ui->actionUpdate_balance,        &QAction::triggered, [this]{m_wallet->updateBalance();});
    connect(ui->actionRefresh_tabs,          &QAction::triggered, [this]{m_wallet->refreshModels();});
    connect(ui->actionRescan_spent,          &QAction::triggered, this, &MainWindow::rescanSpent);
    connect(ui->actionWallet_cache_debug,    &QAction::triggered, this, &MainWindow::showWalletCacheDebugDialog);

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
    connect(ui->actionShow_Searchbar, &QAction::toggled, this, &MainWindow::toggleSearchbar);
    ui->actionShow_Searchbar->setChecked(config()->get(Config::showSearchbar).toBool());

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
    connect(ui->actionAddress_checker,             &QAction::triggered, this, &MainWindow::showAddressChecker);
    connect(ui->actionCalculator,                  &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(ui->actionCreateDesktopEntry,          &QAction::triggered, this, &MainWindow::onCreateDesktopEntry);

    // TODO: Allow creating desktop entry on Windows and Mac
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    ui->actionCreateDesktopEntry->setDisabled(true);
#endif

#ifndef SELF_CONTAINED
    ui->actionCreateDesktopEntry->setVisible(false);
#endif

    // [Help]
    connect(ui->actionAbout,             &QAction::triggered, this, &MainWindow::menuAboutClicked);
#if defined(CHECK_UPDATES)
    connect(ui->actionCheckForUpdates,   &QAction::triggered, this, &MainWindow::showUpdateDialog);
#else
    ui->actionCheckForUpdates->setVisible(false);
#endif

    connect(ui->actionOfficialWebsite,   &QAction::triggered, [this](){Utils::externalLinkWarning(this, "https://featherwallet.org");});
    connect(ui->actionDonate_to_Feather, &QAction::triggered, this, &MainWindow::donateButtonClicked);
    connect(ui->actionDocumentation,     &QAction::triggered, this, &MainWindow::onShowDocumentation);
    connect(ui->actionReport_bug,        &QAction::triggered, this, &MainWindow::onReportBug);
    connect(ui->actionShow_debug_info,   &QAction::triggered, this, &MainWindow::showDebugInfo);


    // Setup shortcuts
    ui->actionStore_wallet->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionRefresh_tabs->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionOpen->setShortcut(QKeySequence("Ctrl+O"));
    ui->actionNew_Restore->setShortcut(QKeySequence("Ctrl+N"));
    ui->actionLock->setShortcut(QKeySequence("Ctrl+L"));
    ui->actionClose->setShortcut(QKeySequence("Ctrl+W"));
    ui->actionShow_debug_info->setShortcut(QKeySequence("Ctrl+D"));
    ui->actionSettings->setShortcut(QKeySequence("Ctrl+Alt+S"));
    ui->actionUpdate_balance->setShortcut(QKeySequence("Ctrl+U"));
    ui->actionShow_Searchbar->setShortcut(QKeySequence("Ctrl+F"));
    ui->actionDocumentation->setShortcut(QKeySequence("F1"));
}

void MainWindow::initHome() {
    // Ticker widgets
    m_tickerWidgets.append(new PriceTickerWidget(this, m_wallet, "XMR"));
    m_tickerWidgets.append(new PriceTickerWidget(this, m_wallet, "BTC"));
    m_tickerWidgets.append(new RatioTickerWidget(this, m_wallet, "XMR", "BTC"));
    for (const auto &widget : m_tickerWidgets) {
        ui->tickerLayout->addWidget(widget);
    }

    m_balanceTickerWidget = new BalanceTickerWidget(this, m_wallet, false);
    ui->fiatTickerLayout->addWidget(m_balanceTickerWidget);

    connect(ui->ccsWidget, &CCSWidget::selected, this, &MainWindow::showSendScreen);
    connect(ui->bountiesWidget, &BountiesWidget::donate, this, &MainWindow::fillSendTab);
    connect(ui->redditWidget, &RedditWidget::setStatusText, this, &MainWindow::setStatusText);
    connect(ui->revuoWidget, &RevuoWidget::donate, [this](const QString &address, const QString &description){
        m_sendWidget->fill(address, description);
        ui->tabWidget->setCurrentIndex(Tabs::SEND);
    });
}

void MainWindow::initWalletContext() {
    connect(m_wallet, &Wallet::balanceUpdated,           this, &MainWindow::onBalanceUpdated);
    connect(m_wallet, &Wallet::synchronized,             this, &MainWindow::onSynchronized); //TODO
    connect(m_wallet, &Wallet::blockchainSync,           this, &MainWindow::onBlockchainSync);
    connect(m_wallet, &Wallet::refreshSync,              this, &MainWindow::onRefreshSync);
    connect(m_wallet, &Wallet::createTransactionError,   this, &MainWindow::onCreateTransactionError);
    connect(m_wallet, &Wallet::createTransactionSuccess, this, &MainWindow::onCreateTransactionSuccess);
    connect(m_wallet, &Wallet::transactionCommitted,     this, &MainWindow::onTransactionCommitted);
    connect(m_wallet, &Wallet::initiateTransaction,      this, &MainWindow::onInitiateTransaction);
    connect(m_wallet, &Wallet::endTransaction,           this, &MainWindow::onEndTransaction);
    connect(m_wallet, &Wallet::keysCorrupted,            this, &MainWindow::onKeysCorrupted);
    connect(m_wallet, &Wallet::selectedInputsChanged,    this, &MainWindow::onSelectedInputsChanged);

    // Wallet
    connect(m_wallet, &Wallet::connectionStatusChanged, [this](int status){
        // Order is important, first inform UI about a potential disconnect, then reconnect
        this->onConnectionStatusChanged(status);
        m_nodes->autoConnect();
    });
    connect(m_wallet, &Wallet::currentSubaddressAccountChanged, this, &MainWindow::updateTitle);
    connect(m_wallet, &Wallet::walletPassphraseNeeded, this, &MainWindow::onWalletPassphraseNeeded);

    connect(m_wallet, &Wallet::unconfirmedMoneyReceived, this, [this](const QString &txId, uint64_t amount){
       if (m_wallet->isSynchronized()) {
           auto notify = QString("%1 XMR (pending)").arg(WalletManager::displayAmount(amount, false));
           Utils::desktopNotify("Payment received", notify, 5000);
       }
    });

    // Device
    connect(m_wallet, &Wallet::deviceButtonRequest, this, &MainWindow::onDeviceButtonRequest);
    connect(m_wallet, &Wallet::deviceButtonPressed, this, &MainWindow::onDeviceButtonPressed);
    connect(m_wallet, &Wallet::deviceError,         this, &MainWindow::onDeviceError);

    connect(m_wallet, &Wallet::donationSent,        this, []{
        config()->set(Config::donateBeg, -1);
    });
    
    connect(m_wallet, &Wallet::multiBroadcast,      this, &MainWindow::onMultiBroadcast);
}

void MainWindow::menuToggleTabVisible(const QString &key){
    const auto toggleTab = m_tabShowHideMapper[key];
    bool show = config()->get(toggleTab->configKey).toBool();
    show = !show;
    config()->set(toggleTab->configKey, show);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
}

void MainWindow::menuClearHistoryClicked() {
    config()->remove(Config::recentlyOpenedWallets);
    this->updateRecentlyOpenedMenu();
}

QString MainWindow::walletName() {
    return QFileInfo(m_wallet->cachePath()).fileName();
}

QString MainWindow::walletCachePath() {
    return m_wallet->cachePath();
}

QString MainWindow::walletKeysPath() {
    return m_wallet->keysPath();
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

    m_wallet->setRingDatabase(Utils::ringDatabasePath());

    m_wallet->updateBalance();
    if (m_wallet->isHwBacked()) {
        m_statusBtnHwDevice->show();
    }

    this->bringToFront();
    this->setEnabled(true);

    // receive page
    m_wallet->subaddress()->refresh(m_wallet->currentSubaddressAccount());
    if (m_wallet->subaddress()->count() == 1) {
        for (int i = 0; i < 10; i++) {
            m_wallet->subaddress()->addRow(m_wallet->currentSubaddressAccount(), "");
        }
    }
    m_wallet->subaddressModel()->setCurrentSubaddressAccount(m_wallet->currentSubaddressAccount());

    // history page
    m_wallet->history()->refresh(m_wallet->currentSubaddressAccount());

    // coins page
    m_wallet->coins()->refresh(m_wallet->currentSubaddressAccount());
    m_coinsWidget->setModel(m_wallet->coinsModel(), m_wallet->coins());
    m_wallet->coinsModel()->setCurrentSubaddressAccount(m_wallet->currentSubaddressAccount());

    // Coin labeling uses set_tx_note, so we need to refresh history too
    connect(m_wallet->coins(), &Coins::descriptionChanged, [this] {
        m_wallet->history()->refresh(m_wallet->currentSubaddressAccount());
    });
    // Vice versa
    connect(m_wallet->history(), &TransactionHistory::txNoteChanged, [this] {
        m_wallet->coins()->refresh(m_wallet->currentSubaddressAccount());
    });

    this->updatePasswordIcon();
    this->updateTitle();
    m_nodes->allowConnection();
    m_nodes->connectToNode();
    m_updateBytes.start(250);

    if (config()->get(Config::writeRecentlyOpenedWallets).toBool()) {
        this->addToRecentlyOpened(m_wallet->cachePath());
    }
}

void MainWindow::onBalanceUpdated(quint64 balance, quint64 spendable) {
    bool hide = config()->get(Config::hideBalance).toBool();
    int displaySetting = config()->get(Config::balanceDisplay).toInt();
    int decimals = config()->get(Config::amountPrecision).toInt();

    QString balance_str = "Balance: ";
    if (hide) {
        balance_str += "HIDDEN";
    }
    else if (displaySetting == Config::totalBalance) {
        balance_str += QString("%1 XMR").arg(WalletManager::displayAmount(balance, false, decimals));
    }
    else if (displaySetting == Config::spendable || displaySetting == Config::spendablePlusUnconfirmed) {
        balance_str += QString("%1 XMR").arg(WalletManager::displayAmount(spendable, false, decimals));

        if (displaySetting == Config::spendablePlusUnconfirmed && balance > spendable) {
            balance_str += QString(" (+%1 XMR unconfirmed)").arg(WalletManager::displayAmount(balance - spendable, false, decimals));
        }
    }

    m_statusLabelBalance->setToolTip("Click for details");
    m_statusLabelBalance->setText(balance_str);
    m_balanceTickerWidget->setHidden(hide);
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

void MainWindow::tryStoreWallet() {
    if (m_wallet->connectionStatus() == Wallet::ConnectionStatus::ConnectionStatus_Synchronizing) {
        QMessageBox::warning(this, "Save wallet", "Unable to save wallet during synchronization.\n\n"
                                                  "Wait until synchronization is finished and try again.");
        return;
    }

    m_wallet->store();
}

void MainWindow::onWebsocketStatusChanged(bool enabled) {
    ui->actionShow_Home->setVisible(enabled);
    ui->actionShow_calc->setVisible(enabled);
    ui->actionShow_Exchange->setVisible(enabled);

    ui->tabWidget->setTabVisible(Tabs::HOME, enabled && config()->get(Config::showTabHome).toBool());
    ui->tabWidget->setTabVisible(Tabs::CALC, enabled && config()->get(Config::showTabCalc).toBool());
    ui->tabWidget->setTabVisible(Tabs::EXCHANGES, enabled && config()->get(Config::showTabExchange).toBool());

    m_historyWidget->setWebsocketEnabled(enabled);
    m_sendWidget->setWebsocketEnabled(enabled);

#ifdef HAS_XMRIG
    m_xmrig->setDownloadsTabEnabled(enabled);
#endif
}

void MainWindow::onProxySettingsChanged() {
    m_nodes->connectToNode();

    int proxy = config()->get(Config::proxy).toInt();

    if (proxy == Config::Proxy::Tor) {
        this->onTorConnectionStateChanged(torManager()->torConnected);
        m_statusBtnProxySettings->show();
        return;
    }

    if (proxy == Config::Proxy::i2p) {
        m_statusBtnProxySettings->setIcon(icons()->icon("i2p.png"));
        m_statusBtnProxySettings->show();
        return;
    }

    m_statusBtnProxySettings->hide();
}

void MainWindow::onOfflineMode(bool offline) {
    if (!m_wallet) {
        return;
    }
    m_wallet->setOffline(offline);
    this->onConnectionStatusChanged(Wallet::ConnectionStatus_Disconnected);
}

void MainWindow::onMultiBroadcast(const QMap<QString, QString> &txHexMap) {
    QMapIterator<QString, QString> i(txHexMap);
    while (i.hasNext()) {
        i.next();
        for (const auto& node: m_nodes->nodes()) {
            QString address = node.toURL();
            qDebug() << QString("Relaying %1 to: %2").arg(i.key(), address);
            m_rpc->setDaemonAddress(address);
            m_rpc->sendRawTransaction(i.value());
        }
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
    if (config()->get(Config::offlineMode).toBool()) {
        icon = icons()->icon("status_offline.svg");
        this->setStatusText("Offline");
    } else {
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
    }

    m_statusBtnConnectionStatusIndicator->setIcon(icon);
}

void MainWindow::onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address) {
    QString err{"Can't create transaction: "};
    if (tx->status() != PendingTransaction::Status_Ok) {
        QString tx_err = tx->errorString();
        qCritical() << tx_err;

        if (m_wallet->connectionStatus() == Wallet::ConnectionStatus_WrongVersion)
            err = QString("%1 Wrong node version: %2").arg(err, tx_err);
        else
            err = QString("%1 %2").arg(err, tx_err);

        if (tx_err.contains("Node response did not include the requested real output")) {
            QString currentNode = m_nodes->connection().toAddress();

            err += QString("\nYou are currently connected to: %1\n\n"
                           "This node may be acting maliciously. You are strongly recommended to disconnect from this node."
                           "Please report this incident to dev@featherwallet.org, #feather on OFTC or /r/FeatherWallet.").arg(currentNode);
        }

        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_wallet->disposeTransaction(tx);
        return;
    }
    else if (tx->txCount() == 0) {
        err = QString("%1 %2").arg(err, "No unmixable outputs to sweep.");
        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_wallet->disposeTransaction(tx);
        return;
    }
    else if (tx->txCount() > 1) {
        err = QString("%1 %2").arg(err, "Split transactions are not supported. Try sending a smaller amount.");
        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_wallet->disposeTransaction(tx);
        return;
    }

    // This is a weak check to see if we send to all specified destination addresses
    // This is here to catch rare memory corruption errors during transaction construction
    // TODO: also check that amounts match
    tx->refresh();
    QSet<QString> outputAddresses;
    for (const auto &output : tx->transaction(0)->outputs()) {
        outputAddresses.insert(WalletManager::baseAddressFromIntegratedAddress(output->address(), constants::networkType));
    }
    QSet<QString> destAddresses;
    for (const auto &addr : address) {
        // TODO: Monero core bug, integrated address is not added to dests for transactions spending ALL
        destAddresses.insert(WalletManager::baseAddressFromIntegratedAddress(addr, constants::networkType));
    }
    if (!outputAddresses.contains(destAddresses)) {
        err = QString("%1 %2").arg(err, "Constructed transaction doesn't appear to send to (all) specified destination address(es). Try creating the transaction again.");
        qDebug() << Q_FUNC_INFO << err;
        this->displayWalletErrorMsg(err);
        m_wallet->disposeTransaction(tx);
        return;
    }

    m_wallet->addCacheTransaction(tx->txid()[0], tx->signedTxToHex(0));

    // Show advanced dialog on multi-destination transactions
    if (address.size() > 1 || m_wallet->viewOnly()) {
        TxConfAdvDialog dialog_adv{m_wallet, m_wallet->tmpTxDescription, this};
        dialog_adv.setTransaction(tx, !m_wallet->viewOnly());
        dialog_adv.exec();
        return;
    }

    TxConfDialog dialog{m_wallet, tx, address[0], m_wallet->tmpTxDescription, this};
    switch (dialog.exec()) {
        case QDialog::Rejected:
        {
            if (!dialog.showAdvanced) {
                m_wallet->disposeTransaction(tx);
            }
            break;
        }
        case QDialog::Accepted:
            m_wallet->commitTransaction(tx, m_wallet->tmpTxDescription);
            break;
    }

    if (dialog.showAdvanced) {
        TxConfAdvDialog dialog_adv{m_wallet, m_wallet->tmpTxDescription, this};
        dialog_adv.setTransaction(tx);
        dialog_adv.exec();
    }
}

void MainWindow::onTransactionCommitted(bool success, PendingTransaction *tx, const QStringList& txid) {
    if (success) {
        QMessageBox msgBox{this};
        QPushButton *showDetailsButton = msgBox.addButton("Show details", QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Ok);
        QString body = QString("Successfully sent %1 transaction(s).").arg(txid.count());
        msgBox.setText(body);
        msgBox.setWindowTitle("Transaction sent");
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.exec();
        if (msgBox.clickedButton() == showDetailsButton) {
            this->showHistoryTab();
            TransactionInfo *txInfo = m_wallet->history()->transaction(txid.first());
            auto *dialog = new TxInfoDialog(m_wallet, txInfo, this);
            connect(dialog, &TxInfoDialog::resendTranscation, this, &MainWindow::onResendTransaction);
            dialog->show();
            dialog->setAttribute(Qt::WA_DeleteOnClose);
        }

        m_sendWidget->clearFields();
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
    WalletInfoDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showSeedDialog() {
    if (m_wallet->isHwBacked()) {
        QMessageBox::information(this, "Information", "Seed unavailable: Wallet keys are stored on hardware device.");
        return;
    }

    if (m_wallet->viewOnly()) {
        QMessageBox::information(this, "Information", "Wallet is view-only and has no seed.\n\nTo obtain wallet keys go to Wallet -> View-Only");
        return;
    }

    if (!m_wallet->isDeterministic()) {
        QMessageBox::information(this, "Information", "Wallet is non-deterministic and has no seed.\n\nTo obtain wallet keys go to Wallet -> Keys");
        return;
    }

    if (!this->verifyPassword()) {
        return;
    }

    SeedDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showPasswordDialog() {
    PasswordChangeDialog dialog{this, m_wallet};
    dialog.exec();
    this->updatePasswordIcon();
}

void MainWindow::updatePasswordIcon() {
    bool emptyPassword = m_wallet->verifyPassword("");
    QIcon icon = emptyPassword ? icons()->icon("unlock.svg") : icons()->icon("lock.svg");
    m_statusBtnPassword->setIcon(icon);
}

void MainWindow::showKeysDialog() {
    if (!this->verifyPassword()) {
        return;
    }

    KeysDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showViewOnlyDialog() {
    ViewOnlyDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::menuHwDeviceClicked() {
    QMessageBox::information(this, "Hardware Device", QString("This wallet is backed by a %1 hardware device.").arg(this->getHardwareDevice()));
}

void MainWindow::menuOpenClicked() {
    m_windowManager->wizardOpenWallet();
}

void MainWindow::menuNewRestoreClicked() {
    m_windowManager->showWizard(WalletWizard::Page_Menu);
}

void MainWindow::menuQuitClicked() {
    this->close();
}

void MainWindow::menuWalletCloseClicked() {
    m_windowManager->showWizard(WalletWizard::Page_Menu);
    this->close();
}

void MainWindow::menuProxySettingsClicked() {
    this->menuSettingsClicked(true);
}

void MainWindow::menuAboutClicked() {
    AboutDialog dialog{this};
    dialog.exec();
}

void MainWindow::menuSettingsClicked(bool showProxyTab) {
    m_windowManager->showSettings(m_nodes, this, showProxyTab);
}

void MainWindow::menuSignVerifyClicked() {
    SignVerifyDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::menuVerifyTxProof() {
    VerifyProofDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::onShowSettingsPage(int page) {
    config()->set(Config::lastSettingsPage, page);
    this->menuSettingsClicked();
}

void MainWindow::skinChanged(const QString &skinName) {
    ColorScheme::updateFromWidget(this);
    this->updateWidgetIcons();
}

void MainWindow::updateWidgetIcons() {
    m_sendWidget->skinChanged();
#ifdef HAS_LOCALMONERO
    m_localMoneroWidget->skinChanged();
#endif
    ui->conversionWidget->skinChanged();
    ui->revuoWidget->skinChanged();

    m_statusBtnHwDevice->setIcon(this->hardwareDevicePairedIcon());
}

QIcon MainWindow::hardwareDevicePairedIcon() {
    QString filename;
    if (m_wallet->isLedger())
        filename = "ledger.png";
    else if (m_wallet->isTrezor())
        filename = ColorScheme::darkScheme ? "trezor_white.png" : "trezor.png";
    return icons()->icon(filename);
}

QIcon MainWindow::hardwareDeviceUnpairedIcon() {
    QString filename;
    if (m_wallet->isLedger())
        filename = "ledger_unpaired.png";
    else if (m_wallet->isTrezor())
        filename = ColorScheme::darkScheme ? "trezor_unpaired_white.png" : "trezor_unpaired.png";
    return icons()->icon(filename);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << Q_FUNC_INFO;

    if (!this->cleanedUp) {
        this->cleanedUp = true;

        config()->set(Config::homeWidget, ui->tabHomeWidget->currentIndex());

        m_historyWidget->resetModel();

        m_updateBytes.stop();
        m_txTimer.stop();

        // Wallet signal may fire after AppContext is gone, causing segv
        m_wallet->disconnect();
        this->disconnect();

        this->saveGeo();
        m_windowManager->closeWindow(this);
    }

    event->accept();
}

void MainWindow::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::WindowStateChange) && this->isMinimized()) {
        if (config()->get(Config::lockOnMinimize).toBool()) {
            this->lockWallet();
        }
    } else {
        QMainWindow::changeEvent(event);
    }
}

void MainWindow::donateButtonClicked() {
    m_sendWidget->fill(constants::donationAddress, "Donation to the Feather development team");
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

void MainWindow::fillSendTab(const QString &address, const QString &description) {
    m_sendWidget->fill(address, description);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::showCalcWindow() {
    m_windowCalc->show();
}

void MainWindow::payToMany() {
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
    m_sendWidget->payToMany();
    QMessageBox::information(this, "Pay to many", "Enter a list of outputs in the 'Pay to' field.\n"
                                                  "One output per line.\n"
                                                  "Format: address, amount\n"
                                                  "A maximum of 16 addresses may be specified.");
}

void MainWindow::showSendScreen(const CCSEntry &entry) { // TODO: rename this function
    m_sendWidget->fill(entry.address, QString("CCS: %1").arg(entry.title));
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::onViewOnBlockExplorer(const QString &txid) {
    QString blockExplorerLink = Utils::blockExplorerLink(config()->get(Config::blockExplorer).toString(), constants::networkType, txid);
    Utils::externalLinkWarning(this, blockExplorerLink);
}

void MainWindow::onResendTransaction(const QString &txid) {
    QString txHex = m_wallet->getCacheTransaction(txid);
    if (txHex.isEmpty()) {
        QMessageBox::warning(this, "Unable to resend transaction", "Transaction was not found in transaction cache. Unable to resend.");
        return;
    }

    // Connect to a different node so chances of successful relay are higher
    m_nodes->autoConnect(true);

    TxBroadcastDialog dialog{this, m_nodes, txHex};
    dialog.exec();
}

void MainWindow::importContacts() {
    const QString targetFile = QFileDialog::getOpenFileName(this, "Import CSV file", QDir::homePath(), "CSV Files (*.csv)");
    if(targetFile.isEmpty()) return;

    auto *model = m_wallet->addressBookModel();
    QMapIterator<QString, QString> i(model->readCSV(targetFile));
    int inserts = 0;
    while (i.hasNext()) {
        i.next();
        bool addressValid = WalletManager::addressValid(i.value(), m_wallet->nettype());
        if(addressValid) {
            m_wallet->addressBook()->addRow(i.value(), "", i.key());
            inserts++;
        }
    }

    QMessageBox::information(this, "Contacts imported", QString("Total contacts imported: %1").arg(inserts));
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
    DebugInfoDialog dialog{m_wallet, m_nodes, this};
    dialog.exec();
}

void MainWindow::showWalletCacheDebugDialog() {
    if (!this->verifyPassword()) {
        return;
    }

    WalletCacheDebugDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showAccountSwitcherDialog() {
    m_accountSwitcherDialog->show();
    m_accountSwitcherDialog->update();
}

void MainWindow::showAddressChecker() {
    QString address = QInputDialog::getText(this, "Address Checker", "Address:                                      ");
    if (address.isEmpty()) {
        return;
    }

    if (!WalletManager::addressValid(address, constants::networkType)) {
        QMessageBox::warning(this, "Address Checker", "Invalid address.");
        return;
    }

    SubaddressIndex index = m_wallet->subaddressIndex(address);
    if (!index.isValid()) {
        // TODO: probably mention lookahead here
        QMessageBox::warning(this, "Address Checker", "This address does not belong to this wallet.");
        return;
    } else {
        QMessageBox::information(this, "Address Checker", QString("This address belongs to Account #%1").arg(index.major));
    }
}

void MainWindow::exportKeyImages() {
    QString fn = QFileDialog::getSaveFileName(this, "Save key images to file", QString("%1/%2_%3").arg(QDir::homePath(), this->walletName(), QString::number(QDateTime::currentSecsSinceEpoch())), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_keyImages")) fn += "_keyImages";
    bool r = m_wallet->exportKeyImages(fn, true);
    if (!r) {
        QMessageBox::warning(this, "Key image export", QString("Failed to export key images.\nReason: %1").arg(m_wallet->errorString()));
    } else {
        QMessageBox::information(this, "Key image export", "Successfully exported key images.");
    }
}

void MainWindow::importKeyImages() {
    QString fn = QFileDialog::getOpenFileName(this, "Import key image file", QDir::homePath(), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    bool r = m_wallet->importKeyImages(fn);
    if (!r) {
        QMessageBox::warning(this, "Key image import", QString("Failed to import key images.\n\n%1").arg(m_wallet->errorString()));
    } else {
        QMessageBox::information(this, "Key image import", "Successfully imported key images");
        m_wallet->refreshModels();
    }
}

void MainWindow::exportOutputs() {
    QString fn = QFileDialog::getSaveFileName(this, "Save outputs to file", QString("%1/%2_%3").arg(QDir::homePath(), this->walletName(), QString::number(QDateTime::currentSecsSinceEpoch())), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_outputs")) fn += "_outputs";
    bool r = m_wallet->exportOutputs(fn, true);
    if (!r) {
        QMessageBox::warning(this, "Outputs export", QString("Failed to export outputs.\nReason: %1").arg(m_wallet->errorString()));
    } else {
        QMessageBox::information(this, "Outputs export", "Successfully exported outputs.");
    }
}

void MainWindow::importOutputs() {
    QString fn = QFileDialog::getOpenFileName(this, "Import outputs file", QDir::homePath(), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    bool r = m_wallet->importOutputs(fn);
    if (!r) {
        QMessageBox::warning(this, "Outputs import", QString("Failed to import outputs.\n\n%1").arg(m_wallet->errorString()));
    } else {
        QMessageBox::information(this, "Outputs import", "Successfully imported outputs");
        m_wallet->refreshModels();
    }
}

void MainWindow::loadUnsignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*unsigned_monero_tx)");
    if (fn.isEmpty()) return;
    UnsignedTransaction *tx = m_wallet->loadTxFile(fn);
    auto err = m_wallet->errorString();
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
    UnsignedTransaction *tx = m_wallet->loadTxFromBase64Str(unsigned_tx);
    auto err = m_wallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load unsigned transaction from clipboard", QString("Failed to load transaction.\n\n%1").arg(err));
        return;
    }

    this->createUnsignedTxDialog(tx);
}

void MainWindow::loadSignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) return;
    PendingTransaction *tx = m_wallet->loadSignedTxFile(fn);
    auto err = m_wallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load signed transaction from file", err);
        return;
    }

    TxConfAdvDialog dialog{m_wallet, "", this};
    dialog.setTransaction(tx);
    dialog.exec();
}

void MainWindow::loadSignedTxFromText() {
    TxBroadcastDialog dialog{this, m_nodes};
    dialog.exec();
}

void MainWindow::createUnsignedTxDialog(UnsignedTransaction *tx) {
    TxConfAdvDialog dialog{m_wallet, "", this};
    dialog.setUnsignedTransaction(tx);
    dialog.exec();
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

    TxImportDialog dialog(this, m_wallet);
    dialog.exec();
}

void MainWindow::onDeviceError(const QString &error) {
    qCritical() << "Device error: " << error;

    if (m_showDeviceError) {
        return;
    }

    m_statusBtnHwDevice->setIcon(this->hardwareDeviceUnpairedIcon());
    while (true) {
        m_showDeviceError = true;
        auto result = QMessageBox::question(this, "Hardware device", "Lost connection to hardware device. Attempt to reconnect?");
        if (result == QMessageBox::Yes) {
            bool r = m_wallet->reconnectDevice();
            if (r) {
                break;
            }
        }
        if (result == QMessageBox::No) {
            this->menuWalletCloseClicked();
            return;
        }
    }
    m_statusBtnHwDevice->setIcon(this->hardwareDevicePairedIcon());
    m_wallet->startRefresh();
    m_showDeviceError = false;
}

void MainWindow::onDeviceButtonRequest(quint64 code) {
    qDebug() << "DeviceButtonRequest, code: " << code;

    if (m_wallet->isTrezor()) {
        switch (code) {
            case 1:
            {
                m_splashDialog->setMessage("Action required on device: Enter your PIN to continue");
                m_splashDialog->setIcon(QPixmap(":/assets/images/key.png"));
                m_splashDialog->show();
                m_splashDialog->setEnabled(true);
                break;
            }
            case 8:
            default:
            {
                // Annoyingly, this code is used for a variety of actions, including:
                // Confirm refresh: Do you really want to start refresh?
                // Confirm export: Do you really want to export tx_key?

                if (m_constructingTransaction) { // This code is also used when signing a tx, we handle this elsewhere
                    break;
                }

                m_splashDialog->setMessage("Confirm action on device to proceed");
                m_splashDialog->setIcon(QPixmap(":/assets/images/confirmed.png"));
                m_splashDialog->show();
                m_splashDialog->setEnabled(true);
                break;
            }
        }
    }
}

void MainWindow::onDeviceButtonPressed() {
    if (m_constructingTransaction) {
        return;
    }

    m_splashDialog->hide();
}

void MainWindow::onWalletPassphraseNeeded(bool on_device) {
    auto button = QMessageBox::question(nullptr, "Wallet Passphrase Needed", "Enter passphrase on hardware wallet?\n\n"
                                                                             "It is recommended to enter passphrase on "
                                                                             "the hardware wallet for better security.",
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (button == QMessageBox::Yes) {
        m_wallet->onPassphraseEntered("", true, false);
        return;
    }

    bool ok;
    QString passphrase = QInputDialog::getText(nullptr, "Wallet Passphrase Needed", "Enter passphrase:", QLineEdit::EchoMode::Password, "", &ok);
    if (ok) {
        m_wallet->onPassphraseEntered(passphrase, false, false);
    } else {
        m_wallet->onPassphraseEntered(passphrase, false, true);
    }
}

void MainWindow::updateNetStats() {
    if (!m_wallet || m_wallet->connectionStatus() == Wallet::ConnectionStatus_Disconnected
                       || m_wallet->connectionStatus() == Wallet::ConnectionStatus_Synchronized)
    {
        m_statusLabelNetStats->hide();
        return;
    }

    m_statusLabelNetStats->show();
    m_statusLabelNetStats->setText(QString("(D: %1)").arg(Utils::formatBytes(m_wallet->getBytesReceived())));
}

void MainWindow::rescanSpent() {
    if (!m_wallet->rescanSpent()) {
        QMessageBox::warning(this, "Rescan spent", m_wallet->errorString());
    } else {
        QMessageBox::information(this, "Rescan spent", "Successfully rescanned spent outputs.");
    }
}

void MainWindow::showBalanceDialog() {
    BalanceDialog dialog{this, m_wallet};
    dialog.exec();
}

QString MainWindow::statusDots() {
    m_statusDots++;
    m_statusDots = m_statusDots % 4;
    return QString(".").repeated(m_statusDots);
}

void MainWindow::showOrHide() {
    if (this->isHidden())
        this->bringToFront();
    else
        this->hide();
}

void MainWindow::bringToFront() {
    ensurePolished();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    raise();
    activateWindow();
}

void MainWindow::onPreferredFiatCurrencyChanged() {
    for (const auto &widget : m_tickerWidgets) {
        widget->updateDisplay();
    }
    m_balanceTickerWidget->updateDisplay();
    m_sendWidget->onPreferredFiatCurrencyChanged();
}

void MainWindow::onHideUpdateNotifications(bool hidden) {
    if (hidden) {
        m_statusUpdateAvailable->hide();
    }
    else if (m_updater->state == Updater::State::UPDATE_AVAILABLE) {
        m_statusUpdateAvailable->show();
    }
}

void MainWindow::onTorConnectionStateChanged(bool connected) {
    if (config()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        return;
    }

    if (connected)
        m_statusBtnProxySettings->setIcon(icons()->icon("tor_logo.png"));
    else
        m_statusBtnProxySettings->setIcon(icons()->icon("tor_logo_disabled.png"));
}

void MainWindow::showUpdateNotification() {
    if (config()->get(Config::hideUpdateNotifications).toBool()) {
        return;
    }

    QString versionDisplay{m_updater->version};
    versionDisplay.replace("beta", "Beta");
    QString updateText = QString("Update to Feather %1 is available").arg(versionDisplay);
    m_statusUpdateAvailable->setText(updateText);
    m_statusUpdateAvailable->setToolTip("Click to Download update.");
    m_statusUpdateAvailable->show();

    m_statusUpdateAvailable->disconnect();
    connect(m_statusUpdateAvailable, &StatusBarButton::clicked, this, &MainWindow::showUpdateDialog);
}

void MainWindow::showUpdateDialog() {
    UpdateDialog updateDialog{this, m_updater};
    connect(&updateDialog, &UpdateDialog::restartWallet, m_windowManager, &WindowManager::restartApplication);
    updateDialog.exec();
}

void MainWindow::onInitiateTransaction() {
    m_statusDots = 0;
    m_constructingTransaction = true;
    m_txTimer.start(1000);

    if (m_wallet->isHwBacked()) {
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

    if (m_wallet->isHwBacked()) {
        m_splashDialog->hide();
    }
}

void MainWindow::onKeysCorrupted() {
    if (!m_criticalWarningShown) {
        m_criticalWarningShown = true;
        QMessageBox::warning(this, "Critical error", "WARNING!\n\nThe wallet keys are corrupted.\n\nTo prevent LOSS OF FUNDS do NOT continue to use this wallet file.\n\nRestore your wallet from seed.\n\nPlease report this incident to the Feather developers.\n\nWARNING!");
        m_sendWidget->disableSendButton();
    }
}

void MainWindow::onSelectedInputsChanged(const QStringList &selectedInputs) {
    int numInputs = selectedInputs.size();

    ui->frame_coinControl->setStyleSheet(ColorScheme::GREEN.asStylesheet(true));
    ui->frame_coinControl->setVisible(numInputs > 0);

    if (numInputs > 0) {
        quint64 totalAmount = 0;
        auto coins = m_wallet->coins()->coinsFromKeyImage(selectedInputs);
        for (const auto coin : coins) {
            totalAmount += coin->amount();
        }

        QString text = QString("Coin control active: %1 selected outputs, %2 XMR").arg(QString::number(numInputs), WalletManager::displayAmount(totalAmount));
        ui->label_coinControl->setText(text);
    }
}

void MainWindow::onExportHistoryCSV(bool checked) {
    if (m_wallet == nullptr)
        return;
    QString fn = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath(), "CSV (*.csv)");
    if (fn.isEmpty())
        return;
    if (!fn.endsWith(".csv"))
        fn += ".csv";
    m_wallet->history()->writeCSV(fn);
    QMessageBox::information(this, "CSV export", QString("Transaction history exported to %1").arg(fn));
}

void MainWindow::onExportContactsCSV(bool checked) {
    if (m_wallet == nullptr) return;
    auto *model = m_wallet->addressBookModel();
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

void MainWindow::onShowDocumentation() {
    Utils::externalLinkWarning(this, "https://docs.featherwallet.org");
}

void MainWindow::onReportBug(bool checked) {
    Utils::externalLinkWarning(this, "https://docs.featherwallet.org/guides/report-an-issue");
}

QString MainWindow::getHardwareDevice() {
    if (!m_wallet->isHwBacked())
        return "";
    if (m_wallet->isTrezor())
        return "Trezor";
    if (m_wallet->isLedger())
        return "Ledger";
    return "Unknown";
}

void MainWindow::updateTitle() {
    QString title = QString("%1 (#%2)").arg(this->walletName(), QString::number(m_wallet->currentSubaddressAccount()));

    if (m_wallet->viewOnly())
        title += " [view-only]";
#ifdef HAS_XMRIG
    if (m_xmrig->isMining())
        title += " [mining]";
#endif

    title += " - Feather";

    this->setWindowTitle(title);
}

void MainWindow::donationNag() {
    if (m_wallet->nettype() != NetworkType::Type::MAINNET)
        return;

    if (m_wallet->viewOnly())
        return;

    if (m_wallet->balanceAll() == 0)
        return;

    auto donationCounter = config()->get(Config::donateBeg).toInt();
    if (donationCounter == -1)
        return;

    donationCounter++;
    if (donationCounter % constants::donationBoundary == 0) {
        auto msg = "Feather is a 100% community-sponsored endeavor. Please consider supporting "
                   "the project financially. Get rid of this message by donating any amount.";
        int ret = QMessageBox::information(this, "Donate to Feather", msg, QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            this->donateButtonClicked();
        }
    }
    config()->set(Config::donateBeg, donationCounter);
}

void MainWindow::addToRecentlyOpened(QString keysFile) {
    auto recent = config()->get(Config::recentlyOpenedWallets).toList();

    if (Utils::isPortableMode()) {
        QDir appPath{Utils::applicationPath()};
        keysFile = appPath.relativeFilePath(keysFile);
    }

    if (recent.contains(keysFile)) {
        recent.removeOne(keysFile);
    }
    recent.insert(0, keysFile);

    QList<QVariant> recent_;
    int count = 0;
    for (const auto &file : recent) {
        if (Utils::fileExists(file.toString())) {
            recent_.append(file);
            count++;
        }
        if (count >= 5) {
            break;
        }
    }

    config()->set(Config::recentlyOpenedWallets, recent_);

    this->updateRecentlyOpenedMenu();
}

void MainWindow::updateRecentlyOpenedMenu() {
    ui->menuRecently_open->clear();
    const QStringList recentWallets = config()->get(Config::recentlyOpenedWallets).toStringList();
    for (const auto &walletPath : recentWallets) {
        QFileInfo fileInfo{walletPath};
        ui->menuRecently_open->addAction(fileInfo.fileName(), m_windowManager, std::bind(&WindowManager::tryOpenWallet, m_windowManager, fileInfo.absoluteFilePath(), ""));
    }
    ui->menuRecently_open->addSeparator();
    ui->menuRecently_open->addAction(m_clearRecentlyOpenAction);
}

bool MainWindow::verifyPassword(bool sensitive) {
    bool incorrectPassword = false;
    while (true) {
        PasswordDialog passwordDialog{this->walletName(), incorrectPassword, sensitive, this};
        int ret = passwordDialog.exec();
        if (ret == QDialog::Rejected) {
            return false;
        }

        if (!m_wallet->verifyPassword(passwordDialog.password)) {
            incorrectPassword = true;
            continue;
        }
        break;
    }
    return true;
}

void MainWindow::userActivity() {
    m_userLastActive = QDateTime::currentSecsSinceEpoch();
}

void MainWindow::closeQDialogChildren(QObject *object) {
    for (QObject *child : object->children()) {
        if (auto *childDlg = dynamic_cast<QDialog*>(child)) {
            qDebug() << "Closing dialog: " << childDlg->objectName();
            childDlg->close();
        }
        this->closeQDialogChildren(child);
    }
}

void MainWindow::checkUserActivity() {
    if (!config()->get(Config::inactivityLockEnabled).toBool()) {
        return;
    }

    if (m_constructingTransaction) {
        return;
    }

    if ((m_userLastActive + (config()->get(Config::inactivityLockTimeout).toInt()*60)) < QDateTime::currentSecsSinceEpoch()) {
        qInfo() << "Locking wallet for inactivity";
        this->lockWallet();
    }
}

void MainWindow::lockWallet() {
    if (m_locked) {
        return;
    }

    if (m_constructingTransaction) {
        QMessageBox::warning(this, "Lock wallet", "Unable to lock wallet during transaction construction");
        return;
    }
    m_walletUnlockWidget->reset();

    // Close all open QDialogs
    this->closeQDialogChildren(this);

    ui->tabWidget->hide();
    this->statusBar()->hide();
    this->menuBar()->hide();
    ui->stackedWidget->setCurrentIndex(1);

    m_checkUserActivity.stop();

    m_locked = true;
}

void MainWindow::unlockWallet(const QString &password) {
    if (!m_locked) {
        return;
    }

    if (!m_wallet->verifyPassword(password)) {
        m_walletUnlockWidget->incorrectPassword();
        return;
    }
    m_walletUnlockWidget->reset();

    ui->tabWidget->show();
    this->statusBar()->show();
    this->menuBar()->show();
    ui->stackedWidget->setCurrentIndex(0);

    m_checkUserActivity.start();

    m_locked = false;
}

void MainWindow::toggleSearchbar(bool visible) {
    config()->set(Config::showSearchbar, visible);

    m_historyWidget->setSearchbarVisible(visible);
    m_receiveWidget->setSearchbarVisible(visible);
    m_contactsWidget->setSearchbarVisible(visible);
    m_coinsWidget->setSearchbarVisible(visible);

    int currentTab = ui->tabWidget->currentIndex();
    if (currentTab == Tabs::HISTORY)
        m_historyWidget->focusSearchbar();
    else if (currentTab == Tabs::SEND)
        m_contactsWidget->focusSearchbar();
    else if (currentTab == Tabs::RECEIVE)
        m_receiveWidget->focusSearchbar();
    else if (currentTab == Tabs::COINS)
        m_coinsWidget->focusSearchbar();
}

MainWindow::~MainWindow() {
    qDebug() << "~MainWindow";
};