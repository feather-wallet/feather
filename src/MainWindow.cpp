// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QCheckBox>

#include "constants.h"
#include "dialog/AddressCheckerIndexDialog.h"
#include "dialog/BalanceDialog.h"
#include "dialog/DebugInfoDialog.h"
#include "dialog/PasswordDialog.h"
#include "dialog/TxBroadcastDialog.h"
#include "dialog/TxConfAdvDialog.h"
#include "dialog/TxConfDialog.h"
#include "dialog/TxImportDialog.h"
#include "dialog/TxInfoDialog.h"
#include "dialog/ViewOnlyDialog.h"
#include "dialog/WalletInfoDialog.h"
#include "dialog/WalletCacheDebugDialog.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/rows/CoinsInfo.h"
#include "libwalletqt/Transfer.h"
#include "plugins/PluginRegistry.h"
#include "utils/AppData.h"
#include "utils/AsyncTask.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/SemanticVersion.h"
#include "utils/TorManager.h"
#include "utils/WebsocketNotifier.h"

#include "wallet/wallet_errors.h"

#ifdef WITH_SCANNER
#include "wizard/offline_tx_signing/OfflineTxSigningWizard.h"
#include "qrcode/scanner/URDialog.h"
#endif

#ifdef CHECK_UPDATES
#include "utils/updater/UpdateDialog.h"
#endif

MainWindow::MainWindow(WindowManager *windowManager, Wallet *wallet, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_windowManager(windowManager)
    , m_wallet(wallet)
    , m_nodes(new Nodes(this, wallet))
    , m_rpc(new DaemonRpc(this, ""))
{
    ui->setupUi(this);

    // Ensure the destructor is called after closeEvent()
    setAttribute(Qt::WA_DeleteOnClose);

    m_splashDialog = new SplashDialog(this);
    m_accountSwitcherDialog = new AccountSwitcherDialog(m_wallet, this);

#ifdef CHECK_UPDATES
    m_updater = QSharedPointer<Updater>(new Updater(this));
#endif

    this->restoreGeo();

    this->initStatusBar();
    this->initPlugins();
    this->initWidgets();
    this->initMenu();
    this->initOffline();
    this->initWalletContext();
    emit uiSetup();

    this->onOfflineMode(conf()->get(Config::offlineMode).toBool());
    conf()->set(Config::restartRequired, false);
    
    // Websocket notifier
#ifdef CHECK_UPDATES
    connect(websocketNotifier(), &WebsocketNotifier::UpdatesReceived, m_updater.data(), &Updater::wsUpdatesReceived);
#endif

    websocketNotifier()->emitCache(); // Get cached data

    connect(m_windowManager, &WindowManager::websocketStatusChanged, this, &MainWindow::onWebsocketStatusChanged);
    this->onWebsocketStatusChanged(!conf()->get(Config::disableWebsocket).toBool());

    connect(m_windowManager, &WindowManager::proxySettingsChanged, this, &MainWindow::onProxySettingsChanged);
    connect(m_windowManager, &WindowManager::updateBalance, m_wallet, &Wallet::updateBalance);
    connect(m_windowManager, &WindowManager::offlineMode, this, &MainWindow::onOfflineMode);

    connect(torManager(), &TorManager::connectionStateChanged, this, &MainWindow::onTorConnectionStateChanged);
    this->onTorConnectionStateChanged(torManager()->torConnected);

#ifdef CHECK_UPDATES
    connect(m_updater.data(), &Updater::updateAvailable, this, &MainWindow::showUpdateNotification);
#endif

    ColorScheme::updateFromWidget(this);
    QTimer::singleShot(1, [this]{this->updateWidgetIcons();});

    // Timers
    connect(&m_updateBytes, &QTimer::timeout, this, &MainWindow::updateNetStats);
    connect(&m_txTimer, &QTimer::timeout, [this]{
        m_statusLabelStatus->setText("Constructing transaction" + this->statusDots());
    });

    conf()->set(Config::firstRun, false);

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

void MainWindow::initPlugins() {
    const QStringList enabledPlugins = conf()->get(Config::enabledPlugins).toStringList();

    for (const auto& plugin_creator : PluginRegistry::getPluginCreators()) {
        Plugin* plugin = plugin_creator();

        if (!PluginRegistry::getInstance().isPluginEnabled(plugin->id())) {
            continue;
        }

        qDebug() << "Initializing plugin: " << plugin->id();
        plugin->initialize(m_wallet, this);
        connect(plugin, &Plugin::setStatusText, this, &MainWindow::setStatusText);
        connect(plugin, &Plugin::fillSendTab, this, &MainWindow::fillSendTab);
        connect(this, &MainWindow::updateIcons, plugin, &Plugin::skinChanged);
        connect(this, &MainWindow::aboutToQuit, plugin, &Plugin::aboutToQuit);
        connect(this, &MainWindow::uiSetup, plugin, &Plugin::uiSetup);

        m_plugins.append(plugin);
    }

    std::sort(m_plugins.begin(), m_plugins.end(), [](Plugin *a, Plugin *b) {
        return a->idx() < b->idx();
    });
}

void MainWindow::initWidgets() {
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
        ui->tabWidget->setCurrentIndex(this->findTab("History"));
    });
    connect(m_contactsWidget, &ContactsWidget::fillAddress, m_sendWidget, &SendWidget::fillAddress);

    // [Coins]
    m_coinsWidget = new CoinsWidget(m_wallet, this);
    ui->coinsWidgetLayout->addWidget(m_coinsWidget);

    // [Plugins..]
    for (auto* plugin : m_plugins) {
        if (!plugin->hasParent()) {
            qDebug() << "Adding tab: " << plugin->displayName();

            if (plugin->insertFirst()) {
                ui->tabWidget->insertTab(0, plugin->tab(), icons()->icon(plugin->icon()), plugin->displayName());
            } else {
                ui->tabWidget->addTab(plugin->tab(), icons()->icon(plugin->icon()), plugin->displayName());
            }

            for (auto* child : m_plugins) {
                if (child->hasParent() && child->parent() == plugin->id()) {
                    plugin->addSubPlugin(child);
                }
            }
        }
    }

    ui->frame_coinControl->setVisible(false);
    connect(ui->btn_resetCoinControl, &QPushButton::clicked, [this]{
       m_wallet->setSelectedInputs({});
    });

    m_walletUnlockWidget = new WalletUnlockWidget(this, m_wallet);
    m_walletUnlockWidget->setWalletName(this->walletName());
    ui->walletUnlockLayout->addWidget(m_walletUnlockWidget);

    connect(m_walletUnlockWidget, &WalletUnlockWidget::closeWallet, this, &MainWindow::close);
    connect(m_walletUnlockWidget, &WalletUnlockWidget::unlockWallet, this, &MainWindow::unlockWallet);

    ui->tabWidget->setCurrentIndex(0);
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

    // [Wallet] -> [History]
    connect(ui->actionExport_CSV, &QAction::triggered, this, &MainWindow::onExportHistoryCSV);

    // [Wallet] -> [Contacts]
    connect(ui->actionExportContactsCSV, &QAction::triggered, this, &MainWindow::onExportContactsCSV);
    connect(ui->actionImportContactsCSV, &QAction::triggered, this, &MainWindow::importContacts);

    // [View]
    m_tabShowHideSignalMapper = new QSignalMapper(this);
    connect(ui->actionShow_Searchbar, &QAction::toggled, this, &MainWindow::toggleSearchbar);
    ui->actionShow_Searchbar->setChecked(conf()->get(Config::showSearchbar).toBool());

    // Show/Hide Coins
    connect(ui->actionShow_Coins, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Coins"] = new ToggleTab(ui->tabCoins, "Coins", "Coins", ui->actionShow_Coins);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Coins, "Coins");

    // Show/Hide Plugins..
    for (const auto &plugin : m_plugins) {
        if (plugin->parent() != "") {
            continue;
        }

        auto* pluginAction = new QAction(QString("Show %1").arg(plugin->displayName()), this);
        ui->menuView->insertAction(plugin->insertFirst() ? ui->actionPlaceholderBegin : ui->actionPlaceholderEnd, pluginAction);
        connect(pluginAction, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
        m_tabShowHideMapper[plugin->displayName()] = new ToggleTab(plugin->tab(), plugin->displayName(), plugin->displayName(), pluginAction);
        m_tabShowHideSignalMapper->setMapping(pluginAction, plugin->displayName());
    }
    ui->actionPlaceholderBegin->setVisible(false);
    ui->actionPlaceholderEnd->setVisible(false);

    QStringList enabledTabs = conf()->get(Config::enabledTabs).toStringList();
    for (const auto &key: m_tabShowHideMapper.keys()) {
        const auto toggleTab = m_tabShowHideMapper.value(key);
        bool show = enabledTabs.contains(key);

        toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    }
    connect(m_tabShowHideSignalMapper, &QSignalMapper::mappedString, this, &MainWindow::menuToggleTabVisible);

    // [Tools]
    connect(ui->actionSignVerify,                  &QAction::triggered, this, &MainWindow::menuSignVerifyClicked);
    connect(ui->actionVerifyTxProof,               &QAction::triggered, this, &MainWindow::menuVerifyTxProof);
    connect(ui->actionKeyImageSync,                &QAction::triggered, this, &MainWindow::showKeyImageSyncWizard);
    connect(ui->actionLoadSignedTxFromFile,        &QAction::triggered, this, &MainWindow::loadSignedTx);
    connect(ui->actionLoadSignedTxFromText,        &QAction::triggered, this, &MainWindow::loadSignedTxFromText);
    connect(ui->actionImport_transaction,          &QAction::triggered, this, &MainWindow::importTransaction);
    connect(ui->actionTransmitOverUR,              &QAction::triggered, this, &MainWindow::showURDialog);
    connect(ui->actionPay_to_many,                 &QAction::triggered, this, &MainWindow::payToMany);
    connect(ui->actionAddress_checker,             &QAction::triggered, this, &MainWindow::showAddressChecker);
    connect(ui->actionCreateDesktopEntry,          &QAction::triggered, this, &MainWindow::onCreateDesktopEntry);

    if (m_wallet->viewOnly()) {
        ui->actionKeyImageSync->setText("Key image sync");
    }

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

void MainWindow::initOffline() {
    // TODO: check if we have any cameras available

    ui->btn_help->setFocusPolicy(Qt::NoFocus);
    ui->btn_viewOnlyDetails->setFocusPolicy(Qt::NoFocus);
    ui->btn_checkAddress->setFocusPolicy(Qt::NoFocus);
    ui->btn_signTransaction->setFocusPolicy(Qt::StrongFocus);
    ui->btn_signTransaction->setFocus();

    connect(ui->btn_help, &QPushButton::clicked, [this] {
        windowManager()->showDocs(this, "offline_tx_signing");
    });
    connect(ui->btn_viewOnlyDetails, &QPushButton::clicked, [this] {
         this->showViewOnlyDialog();
    });
    connect(ui->btn_checkAddress, &QPushButton::clicked, [this]{
        AddressCheckerIndexDialog dialog{m_wallet, this};
        dialog.exec();
    });
    connect(ui->btn_signTransaction, &QPushButton::clicked, [this] {
        this->showKeyImageSyncWizard();
    });

    switch (conf()->get(Config::offlineTxSigningMethod).toInt()) {
        case Config::OTSMethod::FileTransfer:
            ui->radio_airgapFiles->setChecked(true);
            break;
        default:
            ui->radio_airgapUR->setChecked(true);
    }

    // We can't use rich text for radio buttons
    connect(ui->label_airgapUR, &ClickableLabel::clicked, [this] {
        ui->radio_airgapUR->setChecked(true);
    });
    connect(ui->label_airgapFiles, &ClickableLabel::clicked, [this] {
        ui->radio_airgapFiles->setChecked(true);
    });

    connect(ui->radio_airgapFiles, &QCheckBox::toggled, [this] (bool checked){
        if (checked) {
            conf()->set(Config::offlineTxSigningMethod, Config::OTSMethod::FileTransfer);
        }
    });
    connect(ui->radio_airgapUR, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            conf()->set(Config::offlineTxSigningMethod, Config::OTSMethod::UnifiedResources);
        }
    });
}

void MainWindow::initWalletContext() {
    connect(m_wallet, &Wallet::balanceUpdated,           this, &MainWindow::onBalanceUpdated);
    connect(m_wallet, &Wallet::syncStatus,               this, &MainWindow::onSyncStatus);
    connect(m_wallet, &Wallet::transactionCreated,       this, &MainWindow::onTransactionCreated);
    connect(m_wallet, &Wallet::transactionCommitted,     this, &MainWindow::onTransactionCommitted);
    connect(m_wallet, &Wallet::initiateTransaction,      this, &MainWindow::onInitiateTransaction);
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
       if (m_wallet->isSynchronized() && !m_locked) {
           auto notify = QString("%1 XMR (pending)").arg(WalletManager::displayAmount(amount, false));
           m_windowManager->notify("Payment received", notify, 5000);
       }
    });

    // Device
    connect(m_wallet, &Wallet::deviceButtonRequest, this, &MainWindow::onDeviceButtonRequest);
    connect(m_wallet, &Wallet::deviceButtonPressed, this, &MainWindow::onDeviceButtonPressed);
    connect(m_wallet, &Wallet::deviceError,         this, &MainWindow::onDeviceError);

    connect(m_wallet, &Wallet::donationSent,        this, []{
        conf()->set(Config::donateBeg, -1);
    });
    
    connect(m_wallet, &Wallet::multiBroadcast,      this, &MainWindow::onMultiBroadcast);
}

void MainWindow::menuToggleTabVisible(const QString &key){
    const auto toggleTab = m_tabShowHideMapper[key];

    QStringList enabledTabs = conf()->get(Config::enabledTabs).toStringList();
    bool show = enabledTabs.contains(key);
    show = !show;

    if (show) {
        enabledTabs.append(key);
    } else {
        enabledTabs.removeAll(key);
    }

    conf()->set(Config::enabledTabs, enabledTabs);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
}

void MainWindow::menuClearHistoryClicked() {
    conf()->remove(Config::recentlyOpenedWallets);
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
    m_wallet->history()->refresh();

    // coins page
    m_wallet->coins()->refresh();
    m_coinsWidget->setModel(m_wallet->coinsModel(), m_wallet->coins());
    m_wallet->coinsModel()->setCurrentSubaddressAccount(m_wallet->currentSubaddressAccount());

    // Coin labeling uses set_tx_note, so we need to refresh history too
    connect(m_wallet->coins(), &Coins::descriptionChanged, [this] {
        m_wallet->history()->refresh();
    });
    // Vice versa
    connect(m_wallet->history(), &TransactionHistory::txNoteChanged, [this] {
        m_wallet->coins()->refresh();
    });

    this->updatePasswordIcon();
    this->updateTitle();
    m_nodes->allowConnection();
    m_nodes->connectToNode();
    m_updateBytes.start(250);

    if (conf()->get(Config::writeRecentlyOpenedWallets).toBool()) {
        this->addToRecentlyOpened(m_wallet->cachePath());
    }
}

void MainWindow::onBalanceUpdated(quint64 balance, quint64 spendable) {
    bool hide = conf()->get(Config::hideBalance).toBool();
    int displaySetting = conf()->get(Config::balanceDisplay).toInt();
    int decimals = conf()->get(Config::amountPrecision).toInt();

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
        Utils::showError(this, "Unable to save wallet", "Can't save wallet during synchronization", {"Wait until synchronization is finished and try again"}, "synchronization");
        return;
    }

    m_wallet->store();
}

void MainWindow::onWebsocketStatusChanged(bool enabled) {
    ui->actionShow_Home->setVisible(enabled);

    QStringList enabledTabs = conf()->get(Config::enabledTabs).toStringList();

    for (const auto &plugin : m_plugins) {
        if (plugin->hasParent()) {
            continue;
        }

        if (plugin->requiresWebsocket()) {
            // TODO: unload plugins
            ui->tabWidget->setTabVisible(this->findTab(plugin->displayName()), enabled && enabledTabs.contains(plugin->displayName()));
        }
    }

    m_historyWidget->setWebsocketEnabled(enabled);
    m_sendWidget->setWebsocketEnabled(enabled);
}

void MainWindow::onProxySettingsChanged() {
    m_nodes->connectToNode();

    int proxy = conf()->get(Config::proxy).toInt();

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
    m_wallet->setOffline(offline);

    if (m_wallet->viewOnly()) {
        return;
    }

    if (ui->stackedWidget->currentIndex() != Stack::LOCKED) {
        ui->stackedWidget->setCurrentIndex(offline ? Stack::OFFLINE: Stack::WALLET);
    }

    ui->actionPay_to_many->setVisible(!offline);
    ui->menuView->setDisabled(offline);

    m_statusLabelBalance->setVisible(!offline);
    m_statusBtnProxySettings->setVisible(!offline);
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

void MainWindow::onSyncStatus(quint64 height, quint64 target, bool daemonSync) {
    if (height >= (target - 1)) {
        this->updateNetStats();
    }
    this->setStatusText(Utils::formatSyncStatus(height, target, daemonSync));
    m_statusLabelStatus->setToolTip(QString("Wallet height: %1").arg(QString::number(height)));
}

void MainWindow::onConnectionStatusChanged(int status)
{
    // Note: Wallet does not emit this signal unless status is changed, so calling this function from MainWindow may
    // result in the wrong connection status being displayed.

    qDebug() << "Wallet connection status changed " << Utils::QtEnumToString(static_cast<Wallet::ConnectionStatus>(status));

    // Update connection info in status bar.

    QIcon icon;
    if (conf()->get(Config::offlineMode).toBool()) {
        icon = icons()->icon("status_offline.svg");
        this->setStatusText("Offline mode");
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

void MainWindow::onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address) {
    // Clean up some UI
    m_constructingTransaction = false;
    m_txTimer.stop();
    this->setStatusText(m_statusText);

    if (m_wallet->isHwBacked()) {
        m_splashDialog->hide();
    }

    if (tx->status() != PendingTransaction::Status_Ok) {
        if (m_showDeviceError) {
            // The hardware devices has disconnected during tx construction.
            // Due to a macOS-specific Qt bug, we have to prevent it from stacking two QMessageBoxes, otherwise
            // the UI becomes unresponsive. The reconnect dialog should take priority.
            m_wallet->disposeTransaction(tx);
            return;
        }

        QString errMsg = tx->errorString();

        Utils::Message message{this, Utils::ERROR, "Failed to construct transaction", errMsg};

        if (tx->getException()) {
            try
            {
                std::rethrow_exception(tx->getException());
            }
            catch (const tools::error::daemon_busy &e) {
                message.description = QString("Node was unable to respond. Failed request: %1").arg(QString::fromStdString(e.request()));
                message.helpItems = {"Try sending the transaction again.", "If this keeps happening, connect to a different node."};
            }
            catch (const tools::error::no_connection_to_daemon &e) {
                message.description = QString("Connection to node lost. Failed request: %1").arg(QString::fromStdString(e.request()));
                message.helpItems = {"Try sending the transaction again.", "If this keeps happening, connect to a different node."};
            }
            catch (const tools::error::wallet_rpc_error &e) {
                message.description = QString("RPC error: %1").arg(QString::fromStdString(e.to_string()));
                message.helpItems = {"Try sending the transaction again.", "If this keeps happening, connect to a different node."};
            }
            catch (const tools::error::get_outs_error &e) {
                message.description = "Failed to get enough decoy outputs from node";
                message.helpItems = {"Your transaction has too many inputs. Try sending a lower amount."};
            }
            catch (const tools::error::not_enough_unlocked_money &e) {
                QString error;
                if (e.fee() > e.available()) {
                    error = QString("Transaction fee exceeds spendable balance.\n\nSpendable balance: %1\nTransaction fee: %2").arg(WalletManager::displayAmount(e.available()), WalletManager::displayAmount(e.fee()));
                }
                else {
                    error = QString("Spendable balance insufficient to pay for transaction.\n\nSpendable balance: %1\nTransaction needs: %2").arg(WalletManager::displayAmount(e.available()), WalletManager::displayAmount(e.tx_amount() + e.fee()));
                }
                message.description = error;
                message.helpItems = {"Wait for more balance to unlock.", "Click 'Help' to learn more about how balance works."};
                message.doc = "balance";
            }
            catch (const tools::error::not_enough_money &e) {
                message.description = QString("Not enough money to transfer\n\nTotal balance: %1\nTransaction amount: %2").arg(WalletManager::displayAmount(e.available()), WalletManager::displayAmount(e.tx_amount()));
                message.helpItems = {"If you are trying to send your entire balance, click 'Max'."};
                message.doc = "balance";
            }
            catch (const tools::error::tx_not_possible &e) {
                message.description = QString("Not enough money to transfer. Transaction amount + fee exceeds available balance.");
                message.helpItems = {"If you're trying to send your entire balance, click 'Max'."};
                message.doc = "balance";
            }
            catch (const tools::error::not_enough_outs_to_mix &e) {
                message.description = "Not enough outputs for specified ring size.";
            }
            catch (const tools::error::tx_not_constructed&) {
                message.description = "Transaction was not constructed";
                message.helpItems = {"You have found a bug. Please contact the developers."};
                message.doc = "report_an_issue";
            }
            catch (const tools::error::tx_rejected &e) {
                // TODO: provide helptext
                message.description = QString("Transaction was rejected by node. Reason: %1.").arg(QString::fromStdString(e.status()));
            }
            catch (const tools::error::tx_sum_overflow &e) {
                message.description = "Transaction tries to spend an unrealistic amount of XMR";
                message.helpItems = {"You have found a bug. Please contact the developers."};
                message.doc = "report_an_issue";
            }
            catch (const tools::error::zero_amount&) {
                message.description = "Destination amount is zero";
                message.helpItems = {"You have found a bug. Please contact the developers."};
                message.doc = "report_an_issue";
            }
            catch (const tools::error::zero_destination&) {
                message.description = "Transaction has no destination";
                message.helpItems = {"You have found a bug. Please contact the developers."};
                message.doc = "report_an_issue";
            }
            catch (const tools::error::tx_too_big &e) {
                message.description = "Transaction too big";
                message.helpItems = {"Try sending a smaller amount."};
            }
            catch (const tools::error::transfer_error &e) {
                message.description = QString("Unknown transfer error: %1").arg(QString::fromStdString(e.what()));
                message.helpItems = {"You have found a bug. Please contact the developers."};
                message.doc = "report_an_issue";
            }
            catch (const tools::error::wallet_internal_error &e) {
                bool bug = true;
                QString msg = e.what();
                message.description = QString("Internal error: %1").arg(QString::fromStdString(e.what()));

                if (msg.contains("Daemon response did not include the requested real output")) {
                    QString currentNode = m_nodes->connection().toAddress();
                    message.description += QString("\nYou are currently connected to: %1\n\n"
                                                   "This node may be acting maliciously. You are strongly recommended to disconnect from this node."
                                                   "Please report this incident to the developers.").arg(currentNode);
                    message.doc = "report_an_issue";
                }
                if (msg.startsWith("No unlocked balance")) {
                    // TODO: We're sending ALL, but fractional outputs got ignored
                    message.description = "Spendable balance insufficient to pay for transaction fee.";
                    bug = false;
                }

                if (bug) {
                    message.helpItems = {"You have found a bug. Please contact the developers."};
                    message.doc = "report_an_issue";
                }
            }
            catch (const std::exception &e) {
                message.description = QString::fromStdString(e.what());
            }
        }

        Utils::showMsg(message);
        m_wallet->disposeTransaction(tx);
        return;
    }
    else if (tx->txCount() == 0) {
        Utils::showError(this, "Failed to construct transaction", "No transactions were constructed", {"You have found a bug. Please contact the developers."}, "report_an_issue");
        m_wallet->disposeTransaction(tx);
        return;
    }
    else if (tx->txCount() > 1) {
        Utils::showError(this, "Failed to construct transaction", "Split transactions are not supported", {"Try sending a smaller amount."});
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
        Utils::showError(this, "Transaction fails sanity check", "Constructed transaction doesn't appear to send to (all) specified destination address(es). Try creating the transaction again.");
        m_wallet->disposeTransaction(tx);
        return;
    }

    m_wallet->addCacheTransaction(tx->txid()[0], tx->signedTxToHex(0));

    // Offline transaction signing
    if (m_wallet->viewOnly()) {
#ifdef WITH_SCANNER
        OfflineTxSigningWizard wizard(this, m_wallet, tx);
        wizard.exec();
        
        if (!wizard.readyToCommit()) {
            return;
        } else {
            tx = wizard.signedTx();
        }

        if (tx->txCount() == 0) {
            Utils::showError(this, "Failed to load transaction", "No transactions were found", {"You have found a bug. Please contact the developers."}, "report_an_issue");
            m_wallet->disposeTransaction(tx);
            return;
        }
#else
        Utils::showError(this, "Can't open offline transaction signing wizard", "Feather was built without webcam QR scanner support");
        return;
#endif
    }

    // Show advanced dialog on multi-destination transactions
    if (address.size() > 1) {
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
    if (!success) {
        QString error = tx->errorString();
        if (m_wallet->viewOnly() && error.contains("double spend")) {
            m_wallet->setForceKeyImageSync(true);
        }
        if (error.contains("no connection to daemon")) {
            QMessageBox box(this);
            box.setWindowTitle("Question");
            box.setText("Unable to send transaction");
            box.setInformativeText("No connection to node. Retry sending transaction?");
            QPushButton *manual = box.addButton("Broadcast manually", QMessageBox::HelpRole);
            box.addButton(QMessageBox::No);
            box.addButton(QMessageBox::Yes);

            box.exec();

            if (box.clickedButton() == manual) {
                if (txid.empty()) {
                    Utils::showError(this, "Unable to open tx broadcaster", "Cached transaction not found");
                    return;
                }
                this->onResendTransaction(txid[0]);
            }
            else if (box.result() == QMessageBox::Yes) {
                m_wallet->commitTransaction(tx, m_wallet->tmpTxDescription);
            }
            return;
        }
        Utils::showError(this, "Failed to send transaction", error);
        return;
    }

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
        TransactionRow *txInfo = m_wallet->history()->transaction(txid.first());
        auto *dialog = new TxInfoDialog(m_wallet, txInfo, this);
        connect(dialog, &TxInfoDialog::resendTranscation, this, &MainWindow::onResendTransaction);
        dialog->show();
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_sendWidget->clearFields();
}

void MainWindow::showWalletInfoDialog() {
    WalletInfoDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showSeedDialog() {
    if (m_wallet->isHwBacked()) {
        Utils::showInfo(this, "Seed unavailable", "Wallet keys are stored on a hardware device", {}, "show_wallet_seed");
        return;
    }

    if (m_wallet->viewOnly()) {
        Utils::showInfo(this, "Seed unavailable", "Wallet is view-only", {"To obtain your private spendkey go to Wallet -> Keys"}, "show_wallet_seed");
        return;
    }

    if (!m_wallet->isDeterministic()) {
        Utils::showInfo(this, "Seed unavailable", "Wallet is non-deterministic and has no seed",
                        {"To obtain wallet keys go to Wallet -> Keys"}, "show_wallet_seed");
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
    if (!this->verifyPassword()) {
        return;
    }

    ViewOnlyDialog dialog{m_wallet, this};
    dialog.exec();
}

void MainWindow::showKeyImageSyncWizard() {
#ifdef WITH_SCANNER
    OfflineTxSigningWizard wizard{this, m_wallet};
    wizard.exec();
    
    if (wizard.readyToSign()) {
        TxConfAdvDialog dialog{m_wallet, "", this, true};
        dialog.setUnsignedTransaction(wizard.unsignedTransaction());
        auto r = dialog.exec();

        if (r != QDialog::Accepted) {
            return;
        }

        wizard.setStartId(OfflineTxSigningWizard::Page_ExportSignedTx);
        wizard.restart();
        wizard.exec();
    }
#else
    Utils::showError(this, "Can't open offline transaction signing wizard", "Feather was built without webcam QR scanner support");
#endif
}

void MainWindow::menuHwDeviceClicked() {
    Utils::showInfo(this, "Hardware device", QString("This wallet is backed by a %1 hardware device.").arg(this->getHardwareDevice()));
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
    conf()->set(Config::lastSettingsPage, page);
    this->menuSettingsClicked();
}

void MainWindow::skinChanged(const QString &skinName) {
    ColorScheme::updateFromWidget(this);
    this->updateWidgetIcons();
}

void MainWindow::updateWidgetIcons() {
    m_sendWidget->skinChanged();
    emit updateIcons();

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

        emit aboutToQuit();

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
        if (conf()->get(Config::lockOnMinimize).toBool()) {
            this->lockWallet();
        }
    } else {
        QMainWindow::changeEvent(event);
    }
}

void MainWindow::donateButtonClicked() {
    m_sendWidget->fill(constants::donationAddress, constants::donationDescription);
    ui->tabWidget->setCurrentIndex(this->findTab("Send"));
}

void MainWindow::showHistoryTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(this->findTab("History"));
}

void MainWindow::fillSendTab(const QString &address, const QString &description) {
    m_sendWidget->fill(address, description);
    ui->tabWidget->setCurrentIndex(this->findTab("Send"));
}

void MainWindow::payToMany() {
    ui->tabWidget->setCurrentIndex(this->findTab("Send"));
    m_sendWidget->payToMany();
    Utils::showInfo(this, "Pay to many", "Enter a list of outputs in the 'Pay to' field.\n"
                                         "One output per line.\n"
                                         "Format: address, amount\n"
                                         "A maximum of 16 addresses may be specified.");
}

void MainWindow::onViewOnBlockExplorer(const QString &txid) {
    QString blockExplorerLink = Utils::blockExplorerLink(txid);
    if (blockExplorerLink.isEmpty()) {
        Utils::showError(this, "Unable to open block explorer", "No block explorer configured", {"Go to Settings -> Misc -> Block explorer"});
        return;
    }
    Utils::externalLinkWarning(this, blockExplorerLink);
}

void MainWindow::onResendTransaction(const QString &txid) {
    QString txHex = m_wallet->getCacheTransaction(txid);
    if (txHex.isEmpty()) {
        Utils::showError(this, "Unable to resend transaction", "Transaction was not found in the transaction cache.");
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
            m_wallet->addressBook()->addRow(i.value(), i.key());
            inserts++;
        }
    }

    Utils::showInfo(this, "Contacts imported", QString("Total contacts imported: %1").arg(inserts));
}

void MainWindow::saveGeo() {
    conf()->set(Config::geometry, QString(saveGeometry().toBase64()));
    conf()->set(Config::windowState, QString(saveState().toBase64()));
}

void MainWindow::restoreGeo() {
    bool geo = this->restoreGeometry(QByteArray::fromBase64(conf()->get(Config::geometry).toByteArray()));
    bool windowState = this->restoreState(QByteArray::fromBase64(conf()->get(Config::windowState).toByteArray()));
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
        Utils::showInfo(this, "Invalid address", "The address you entered is not a valid XMR address for the current network type.");
        return;
    }

    SubaddressIndex index = m_wallet->subaddressIndex(address);
    if (!index.isValid()) {
        // TODO: probably mention lookahead here
        Utils::showInfo(this, "This address does not belong to this wallet", "");
        return;
    } else {
        Utils::showInfo(this, QString("This address belongs to Account #%1").arg(index.major));
    }
}

void MainWindow::showURDialog() {
#ifdef WITH_SCANNER
    URDialog dialog{this};
    dialog.exec();
#else
    Utils::showError(this, "Unable to open UR dialog", "Feather was built without webcam scanner support");
#endif
}

void MainWindow::loadSignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*signed_monero_tx);;All Files (*)");
    if (fn.isEmpty()) return;
    PendingTransaction *tx = m_wallet->loadSignedTxFile(fn);
    auto err = m_wallet->errorString();
    if (!err.isEmpty()) {
        Utils::showError(this, "Unable to load signed transaction", err);
        return;
    }

    TxConfAdvDialog dialog{m_wallet, "", this, true};
    dialog.setTransaction(tx);
    dialog.exec();
}

void MainWindow::loadSignedTxFromText() {
    TxBroadcastDialog dialog{this, m_nodes};
    dialog.exec();
}

void MainWindow::importTransaction() {
    if (conf()->get(Config::torPrivacyLevel).toInt() == Config::allTorExceptNode) {
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
                m_splashDialog->setIcon(QPixmap(":/assets/images/confirmed.svg"));
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
    QMessageBox warning{this};
    warning.setWindowTitle("Warning");
    warning.setText("Rescanning spent outputs reveals which outputs you own to the node. "
                    "Make sure you are connected to a trusted node.\n\n"
                    "Do you want to proceed?");
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    
    auto r = warning.exec();
    if (r == QMessageBox::No) {
        return;
    }
    
    if (!m_wallet->rescanSpent()) {
        Utils::showError(this, "Failed to rescan spent outputs", m_wallet->errorString());
    } else {
        Utils::showInfo(this, "Successfully rescanned spent outputs");
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
    m_sendWidget->onPreferredFiatCurrencyChanged();
}

void MainWindow::onHideUpdateNotifications(bool hidden) {
    if (hidden) {
        m_statusUpdateAvailable->hide();
    }
#ifdef CHECK_UPDATES
    else if (m_updater->state == Updater::State::UPDATE_AVAILABLE) {
        m_statusUpdateAvailable->show();
    }
#endif
}

void MainWindow::onTorConnectionStateChanged(bool connected) {
    if (conf()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        return;
    }

    if (connected)
        m_statusBtnProxySettings->setIcon(icons()->icon("tor_logo.png"));
    else
        m_statusBtnProxySettings->setIcon(icons()->icon("tor_logo_disabled.png"));
}

void MainWindow::showUpdateNotification() {
#ifdef CHECK_UPDATES
    if (conf()->get(Config::hideUpdateNotifications).toBool()) {
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
#endif
}

void MainWindow::showUpdateDialog() {
#ifdef CHECK_UPDATES
    UpdateDialog updateDialog{this, m_updater};
    connect(&updateDialog, &UpdateDialog::restartWallet, m_windowManager, &WindowManager::restartApplication);
    updateDialog.exec();
#endif
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

void MainWindow::onKeysCorrupted() {
    if (!m_criticalWarningShown) {
        m_criticalWarningShown = true;
        Utils::showError(this, "Wallet keys are corrupted", "WARNING!\n\nTo prevent LOSS OF FUNDS do NOT continue to use this wallet file.\n\nRestore your wallet from seed.\n\nPlease report this incident to the Feather developers.\n\nWARNING!");
        m_sendWidget->disallowSending();
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

void MainWindow::onExportHistoryCSV() {
    QString fn = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath(), "CSV (*.csv)");
    if (fn.isEmpty())
        return;
    if (!fn.endsWith(".csv"))
        fn += ".csv";
    m_wallet->history()->writeCSV(fn);
    Utils::showInfo(this, "CSV export", QString("Transaction history exported to %1").arg(fn));
}

void MainWindow::onExportContactsCSV() {
    auto *model = m_wallet->addressBookModel();
    if (model->rowCount() <= 0){
        Utils::showInfo(this, "Unable to export contacts", "No contacts to export");
        return;
    }

    const QString targetDir = QFileDialog::getExistingDirectory(this, "Select CSV output directory ", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if(targetDir.isEmpty()) return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QString fn = QString("%1/monero-contacts_%2.csv").arg(targetDir, QString::number(now / 1000));
    if (model->writeCSV(fn)) {
        Utils::showInfo(this, "Contacts exported successfully", QString("Exported to: %1").arg(fn));
    }
}

void MainWindow::onCreateDesktopEntry() {
    auto msg = Utils::xdgDesktopEntryRegister() ? "Desktop entry created" : "Desktop entry not created due to an error.";
    QMessageBox::information(this, "Desktop entry", msg);
}

void MainWindow::onShowDocumentation() {
    // TODO: welcome page
    m_windowManager->showDocs(this);
}

void MainWindow::onReportBug() {
    m_windowManager->showDocs(this, "report_an_issue");
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

    if (m_wallet->viewOnly()) {
        title += " [view-only]";
    }

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

    auto donationCounter = conf()->get(Config::donateBeg).toInt();
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
    conf()->set(Config::donateBeg, donationCounter);
}

void MainWindow::addToRecentlyOpened(QString keysFile) {
    auto recent = conf()->get(Config::recentlyOpenedWallets).toList();

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

    conf()->set(Config::recentlyOpenedWallets, recent_);

    this->updateRecentlyOpenedMenu();
}

void MainWindow::updateRecentlyOpenedMenu() {
    ui->menuRecently_open->clear();
    const QStringList recentWallets = conf()->get(Config::recentlyOpenedWallets).toStringList();
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
    if (!conf()->get(Config::inactivityLockEnabled).toBool()) {
        return;
    }

    if (m_constructingTransaction) {
        return;
    }

    if ((m_userLastActive + (conf()->get(Config::inactivityLockTimeout).toInt()*60)) < QDateTime::currentSecsSinceEpoch()) {
        qInfo() << "Locking wallet for inactivity";
        this->lockWallet();
    }
}

void MainWindow::lockWallet() {
    if (m_locked) {
        return;
    }

    if (m_constructingTransaction) {
        Utils::showError(this, "Unable to lock wallet", "Can't lock wallet during transaction construction");
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
    this->onOfflineMode(conf()->get(Config::offlineMode).toBool());

    m_checkUserActivity.start();

    m_locked = false;
}

void MainWindow::toggleSearchbar(bool visible) {
    conf()->set(Config::showSearchbar, visible);

    m_historyWidget->setSearchbarVisible(visible);
    m_receiveWidget->setSearchbarVisible(visible);
    m_contactsWidget->setSearchbarVisible(visible);
    m_coinsWidget->setSearchbarVisible(visible);

    int currentTab = ui->tabWidget->currentIndex();
    if (currentTab == this->findTab("History"))
        m_historyWidget->focusSearchbar();
    else if (currentTab == this->findTab("Send"))
        m_contactsWidget->focusSearchbar();
    else if (currentTab == this->findTab("Receive"))
        m_receiveWidget->focusSearchbar();
    else if (currentTab == this->findTab("Coins"))
        m_coinsWidget->focusSearchbar();
}

int MainWindow::findTab(const QString &title) {
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == title) {
            return i;
        }
    }
    return -1;
}

MainWindow::~MainWindow() {
    qDebug() << "~MainWindow";
}