// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QPixmap>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QFileDialog>
#include <QInputDialog>

#include "mainwindow.h"
#include "widgets/ccswidget.h"
#include "widgets/redditwidget.h"
#include "dialog/txconfdialog.h"
#include "dialog/debuginfodialog.h"
#include "dialog/walletinfodialog.h"
#include "dialog/torinfodialog.h"
#include "utils/utils.h"
#include "utils/config.h"
#include "components.h"
#include "calcwindow.h"
#include "ui_mainwindow.h"

// libwalletqt
#include "libwalletqt/WalletManager.h"
#include "Wallet.h"
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/SubaddressAccount.h"
#include "libwalletqt/AddressBook.h"
#include "model/SubaddressAccountModel.h"
#include "model/SubaddressModel.h"
#include "utils/networktype.h"

MainWindow * MainWindow::pMainWindow = nullptr;

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ctx(ctx),
    m_aboutDialog(new AboutDialog(this)),
    m_windowCalc(new CalcWindow(this))
{
    pMainWindow = this;
    ui->setupUi(this);

    m_windowSettings = new Settings(this);

    // light/dark theme
    m_skins.insert("Native", "");
    this->loadSkins();
    QString skin = config()->get(Config::skin).toString();
    qApp->setStyleSheet(m_skins[skin]);

    this->screenDpiRef = 128;
    this->screenGeo = QApplication::primaryScreen()->availableGeometry();
    this->screenRect = QGuiApplication::primaryScreen()->geometry();
    this->screenDpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    this->screenDpiPhysical = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    this->screenRatio = this->screenDpiPhysical / this->screenDpiRef;
    qInfo() << QString("%1x%2 (%3 DPI)").arg(this->screenRect.width()).arg(this->screenRect.height()).arg(this->screenDpi);

    this->restoreGeo();

    this->create_status_bar();

    // Bootstrap Tor/websockets
    m_ctx->initTor();
    m_ctx->initWS();

    // update statusbar
    connect(m_ctx->tor, &Tor::connectionStateChanged, [this](bool connected){
        connected ? m_statusBtnTor->setIcon(QIcon(":/assets/images/tor_logo.png"))
                  : m_statusBtnTor->setIcon(QIcon(":/assets/images/tor_logo_disabled.png"));});
    connect(m_ctx->nodes, &Nodes::updateStatus, [=](const QString &msg){
        m_statusLabelStatus->setText(msg);
    });

    // menu connects
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::menuQuitClicked);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::menuSettingsClicked);
    connect(ui->actionCalculator, &QAction::triggered, this, &MainWindow::showCalcWindow);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    ui->actionCreateDesktopEntry->setDisabled(true);
#endif
    connect(ui->actionCreateDesktopEntry, &QAction::triggered, [=]{
        auto msg = Utils::xdgDesktopEntryRegister() ? "Desktop entry created" : "Desktop entry not created due to an error.";
        QMessageBox::information(this, "Desktop entry", msg);
    });
    connect(ui->actionReport_bug, &QAction::triggered, [this](){
        QMessageBox::information(this, "Reporting Bugs",
                                 "<body>Please report any bugs as issues on our git repo:<br>\n"
                                 "<a href=\"https://git.wownero.com/feather/feather/issues\" style=\"color: #33A4DF\">https://git.wownero.com/feather/feather/issues</a><br/><br/>"
                                 "\n"
                                 "Before reporting a bug, upgrade to the most recent version of Feather "
                                 "(latest release or git HEAD), and include the version number in your report. "
                                 "Try to explain not only what the bug is, but how it occurs.</body>");
    });
    connect(ui->actionShow_debug_info, &QAction::triggered, this, &MainWindow::showDebugInfo);
    connect(ui->actionOfficialWebsite, &QAction::triggered, [=] { Utils::externalLinkWarning("https://featherwallet.org"); });
    connect(ui->actionAbout_Qt, &QAction::triggered, [=] { QApplication::aboutQt(); });


#if defined(XMRTO)
    // xmr.to connects/widget
    connect(ui->xmrToWidget, &XMRToWidget::viewOrder, m_ctx->XMRTo, &XmrTo::onViewOrder);
    connect(ui->xmrToWidget, &XMRToWidget::getRates, m_ctx->XMRTo, &XmrTo::onGetRates);
    connect(ui->xmrToWidget, &XMRToWidget::createOrder, m_ctx->XMRTo, &XmrTo::createOrder);
    connect(m_ctx->XMRTo, &XmrTo::ratesUpdated, ui->xmrToWidget, &XMRToWidget::onRatesUpdated);
    connect(m_ctx->XMRTo, &XmrTo::connectionError, ui->xmrToWidget, &XMRToWidget::onConnectionError);
    connect(m_ctx->XMRTo, &XmrTo::connectionSuccess, ui->xmrToWidget, &XMRToWidget::onConnectionSuccess);
    connect(m_ctx, &AppContext::balanceUpdated, ui->xmrToWidget, &XMRToWidget::onBalanceUpdated);
    connect(m_ctx->XMRTo, &XmrTo::openURL, this, [=](const QString &url){ Utils::externalLinkWarning(url); });
    ui->xmrToWidget->setHistoryModel(m_ctx->XMRTo->tableModel);
#else
    ui->tabWidget->setTabVisible(5, false);
#endif


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

    // @TODO: only init tray *after* boot
    connect(m_trayActionCalc, &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(m_trayActionSend, &QAction::triggered, this, &MainWindow::showSendTab);
    connect(m_trayActionHistory, &QAction::triggered, this, &MainWindow::showHistoryTab);
    connect(m_trayActionExit, &QAction::triggered, this, &QMainWindow::close);
#endif

    // ticker widgets
    m_tickerWidgets.append(new TickerWidget(this, "XMR"));
    m_tickerWidgets.append(new TickerWidget(this, "BTC"));
    for(auto tickerWidget: m_tickerWidgets) {
        ui->tickerLayout->addWidget(tickerWidget);
    }

    m_balanceWidget = new TickerWidget(this, "XMR", "Balance", true);
    ui->fiatTickerLayout->addWidget(m_balanceWidget);

    // Send widget
    connect(ui->sendWidget, &SendWidget::createTransaction, m_ctx, QOverload<const QString &, const double, const QString &, bool>::of(&AppContext::onCreateTransaction));

    // Nodes
    connect(m_ctx->nodes, &Nodes::nodeExhausted, this, &MainWindow::showNodeExhaustedMessage);
    connect(m_ctx->nodes, &Nodes::WSNodeExhausted, this, &MainWindow::showWSNodeExhaustedMessage);

    // CCS/Reddit widget
    m_ccsWidget = new CCSWidget(this);
    m_redditWidget = new RedditWidget(this);

    m_ccsWidget->hide();
    m_redditWidget->hide();

    ui->coolLayout->addWidget(m_ccsWidget);
    ui->coolLayout->addWidget(m_redditWidget);

    connect(m_ctx, &AppContext::ccsEmpty, [=] {
        if(m_ccsWidget->isVisible()) {
            // display Reddit widget instead
            m_ccsWidget->show();
            m_redditWidget->show();
        }
    });
    connect(m_ctx, &AppContext::ccsUpdated, m_ccsWidget->model(), &CCSModel::updateEntries);
    connect(m_ctx, &AppContext::redditUpdated, m_redditWidget->model(), &RedditModel::updatePosts);
    connect(m_ccsWidget, &CCSWidget::selected, this, &MainWindow::showSendScreen);

    connect(m_ctx, &AppContext::donationNag, [=]{
        auto msg = "Feather is a 100% community-sponsored endeavor. Please consider supporting "
                   "the project financially. Get rid of this message by donating any amount.";
        int ret = QMessageBox::information(this, "Donate to Feather", msg, QMessageBox::Yes, QMessageBox::No);
        switch (ret) {
            case QMessageBox::Yes:
                this->donateButtonClicked();
            case QMessageBox::No:
                break;
            default:
                return;
        }
    });

    // libwalletqt
    connect(this, &MainWindow::walletClosed, ui->xmrToWidget, &XMRToWidget::onWalletClosed);
    connect(this, &MainWindow::walletClosed, ui->sendWidget, &SendWidget::onWalletClosed);
    connect(m_ctx, &AppContext::balanceUpdated, this, &MainWindow::onBalanceUpdated);
    connect(m_ctx, &AppContext::balanceUpdated, ui->xmrToWidget, &XMRToWidget::onBalanceUpdated);
    connect(m_ctx, &AppContext::walletOpened, this, &MainWindow::onWalletOpened);
    connect(m_ctx, &AppContext::walletClosed, this, QOverload<>::of(&MainWindow::onWalletClosed));
    connect(m_ctx, &AppContext::walletOpenedError, this, &MainWindow::onWalletOpenedError);
    connect(m_ctx, &AppContext::walletCreatedError, this, &MainWindow::onWalletCreatedError);
    connect(m_ctx, &AppContext::walletCreated, this, &MainWindow::onWalletCreated);
    connect(m_ctx, &AppContext::synchronized, this, &MainWindow::onSynchronized);
    connect(m_ctx, &AppContext::blockchainSync, this, &MainWindow::onBlockchainSync);
    connect(m_ctx, &AppContext::refreshSync, this, &MainWindow::onRefreshSync);
    connect(m_ctx, &AppContext::createTransactionError, this, &MainWindow::onCreateTransactionError);
    connect(m_ctx, &AppContext::createTransactionSuccess, this, &MainWindow::onCreateTransactionSuccess);
    connect(m_ctx, &AppContext::transactionCommitted, this, &MainWindow::onTransactionCommitted);
    connect(m_ctx, &AppContext::walletOpenPasswordNeeded, this, &MainWindow::onWalletOpenPasswordRequired);

    // Send
    connect(m_ctx, &AppContext::initiateTransaction, ui->sendWidget, &SendWidget::onInitiateTransaction);
    connect(m_ctx, &AppContext::endTransaction, ui->sendWidget, &SendWidget::onEndTransaction);

    // XMR.to
    connect(m_ctx, &AppContext::initiateTransaction, ui->xmrToWidget, &XMRToWidget::onInitiateTransaction);
    connect(m_ctx, &AppContext::endTransaction, ui->xmrToWidget, &XMRToWidget::onEndTransaction);

    // testnet/stagenet warning
    auto worthlessWarning = QString("Feather wallet is currently running in %1 mode. This is meant "
                                    "for developers only. Your coins are WORTHLESS.");
    if(m_ctx->networkType == NetworkType::STAGENET) {
        if (config()->get(Config::warnOnStagenet).toBool()) {
            Utils::showMessageBox("Warning", worthlessWarning.arg("stagenet"), true);
            config()->set(Config::warnOnStagenet, false);
        }
    }
    else if(m_ctx->networkType == NetworkType::TESTNET){
        if (config()->get(Config::warnOnTestnet).toBool()) {
            Utils::showMessageBox("Warning", worthlessWarning.arg("testnet"), true);
            config()->set(Config::warnOnTestnet, false);
        }
    }

    if(config()->get(Config::warnOnAlpha).toBool()) {
        auto alphaWarning = "Feather wallet is currently in alpha. Bugs are to be expected and "
                            "can be reported on our Git repository or on IRC.";
        Utils::showMessageBox("Warning", alphaWarning, true);
        config()->set(Config::warnOnAlpha, false);
    }

    // settings connects
    // Update ticker widget(s) on home tab when settings preferred fiat currency is changed
    for(auto tickerWidget: m_tickerWidgets)
        connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, tickerWidget, &TickerWidget::init);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, m_balanceWidget, &TickerWidget::init);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, m_ctx, &AppContext::onPreferredFiatCurrencyChanged);

    // CCS/Reddit widget
    connect(m_windowSettings, &Settings::homeWidgetChanged, this, &MainWindow::homeWidgetChanged);

    // Skin
    connect(m_windowSettings, &Settings::skinChanged, this, &MainWindow::skinChanged);

    // Wizard
    connect(this, &MainWindow::closed, [=]{
        if(m_wizard != nullptr)
            m_wizard->close();
    });

    // Receive
    connect(ui->receiveWidget, &ReceiveWidget::generateSubaddress, [=]() {
        m_ctx->currentWallet->subaddress()->addRow( m_ctx->currentWallet->currentSubaddressAccount(), "");
        m_ctx->storeWallet();
    });
    connect(ui->receiveWidget, &ReceiveWidget::showTransactions, [this](const QString &text) {
        ui->historyWidget->setSearchText(text);
        ui->tabWidget->setCurrentIndex(1); //history
    });

    // History
    connect(ui->historyWidget, &HistoryWidget::spendProof, [&](const QString &txid){
        QString spendProof = m_ctx->currentWallet->getSpendProof(txid, "");
        Utils::copyToClipboard(spendProof);
    });
    connect(ui->historyWidget, &HistoryWidget::viewOnBlockExplorer, this, &MainWindow::onViewOnBlockExplorer);

    // Contacts
    connect(ui->contactWidget, &ContactsWidget::addContact, this, &MainWindow::onAddContact);
    connect(ui->contactWidget, &ContactsWidget::fillAddress, ui->sendWidget, &SendWidget::fillAddress);

    // Open alias
    connect(ui->sendWidget, &SendWidget::resolveOpenAlias, m_ctx, &AppContext::onOpenAliasResolve);
    connect(m_ctx, &AppContext::openAliasResolveError, ui->sendWidget, &SendWidget::onOpenAliasResolveError);
    connect(m_ctx, &AppContext::openAliasResolved, ui->sendWidget, &SendWidget::onOpenAliasResolved);

    // Coins
    connect(ui->coinsWidget, &CoinsWidget::freeze, [=](int index) {
        m_ctx->currentWallet->coins()->freeze(index);
        m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
        m_ctx->updateBalance();
        // subaddress account filtering should be done in Model maybe, so we can update data in coins() directly
    });
    connect(ui->coinsWidget, &CoinsWidget::freezeMulti, [&](const QVector<int>& indexes) {
        for (int i : indexes) {
            m_ctx->currentWallet->coins()->freeze(i);
            m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
            m_ctx->updateBalance();
        }
    });
    connect(ui->coinsWidget, &CoinsWidget::thaw, [=](int index) {
        m_ctx->currentWallet->coins()->thaw(index);
        m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
        m_ctx->updateBalance();
    });
    connect(ui->coinsWidget, &CoinsWidget::thawMulti, [&](const QVector<int>& indexes) {
        for (int i : indexes) {
            m_ctx->currentWallet->coins()->thaw(i);
            m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
            m_ctx->updateBalance();
        }
    });
    connect(ui->coinsWidget, &CoinsWidget::sweepOutput, m_ctx, &AppContext::onSweepOutput);

    connect(m_ctx, &AppContext::walletClosing, [=]{
        ui->tabWidget->setCurrentIndex(0);
    });
    // setup some UI
    this->initMain();
    this->initWidgets();
    this->initMenu();
}

void MainWindow::initMain() {
    // show wizards or open wallet directly based on cmdargs
    if(m_ctx->cmdargs->isSet("password"))
        m_ctx->walletPassword = m_ctx->cmdargs->value("password");

    QString openPath = "";
    QString autoPath = config()->get(Config::autoOpenWalletPath).toString();
    if(m_ctx->cmdargs->isSet("wallet-file"))
        openPath = m_ctx->cmdargs->value("wallet-file");
    else if(!autoPath.isEmpty())
        if (autoPath.startsWith(QString::number(m_ctx->networkType)))
            openPath = autoPath.remove(0, 1);

    if(!openPath.isEmpty() && Utils::fileExists(openPath)) {
        this->show();
        this->setEnabled(true);

        m_ctx->onOpenWallet(openPath, m_ctx->walletPassword);
        return;
    }

    this->setEnabled(false);
    this->show();
    m_wizard = this->createWizard(WalletWizard::Page_Menu);
    m_wizard->show();
}

void MainWindow::initMenu() {
    // hide/show tabs
    m_tabShowHideSignalMapper = new QSignalMapper(this);

    connect(ui->actionShow_Coins, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Coins"] = new ToggleTab(ui->tabCoins, "Coins", "Coins", ui->actionShow_Coins, Config::showTabCoins);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Coins, "Coins");
#if defined(XMRTO)
    connect(ui->actionShow_xmr_to, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["XMRto"] = new ToggleTab(ui->tabXmrTo, "XMRto", "xmr.to", ui->actionShow_xmr_to, Config::showTabXMRto);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_xmr_to, "XMRto");
#else
    ui->actionShow_xmr_to->setVisible(false);
#endif
    connect(ui->actionShow_calc, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Calc"] = new ToggleTab(ui->tabCalc, "Calc", "Calc", ui->actionShow_calc, Config::showTabCalc);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_calc, "Calc");

    for (const auto &key: m_tabShowHideMapper.keys()) {
        const auto toggleTab = m_tabShowHideMapper.value(key);
        const bool show = config()->get(toggleTab->configKey).toBool();
        toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    }
    connect(m_tabShowHideSignalMapper, &QSignalMapper::mappedString, this, &MainWindow::menuToggleTabVisible);

    // Wallet
    connect(ui->actionInformation, &QAction::triggered, this, &MainWindow::showWalletInfoDialog);
    connect(ui->actionSeed, &QAction::triggered, this, &MainWindow::showSeedDialog);
    connect(ui->actionPassword, &QAction::triggered, this, &MainWindow::showPasswordDialog);
    connect(ui->actionKeys, &QAction::triggered, this, &MainWindow::showKeysDialog);
    connect(ui->actionStore_wallet, &QAction::triggered, [&]{
        m_ctx->currentWallet->store();
    });
    connect(ui->actionRefresh_tabs, &QAction::triggered, [&]{
        m_ctx->refreshModels();
    });
    connect(ui->actionUpdate_balance, &QAction::triggered, [&]{
        m_ctx->updateBalance();
    });

    // set restore height
    connect(ui->actionChange_restore_height, &QAction::triggered, this, &MainWindow::showRestoreHeightDialog);
    connect(m_ctx, &AppContext::customRestoreHeightSet, [=](unsigned int height){
        auto msg = QString("The restore height for this wallet has been set to %1. "
                   "Please re-open the wallet. Feather will now quit.").arg(height);
        QMessageBox::information(this, "Cannot set custom restore height", msg);
        this->menuQuitClicked();
    });

    // CSV tx export
    connect(ui->actionExport_CSV, &QAction::triggered, [=]{
        if(m_ctx->currentWallet == nullptr) return;
        QString fn = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath(), "CSV (*.csv)");
        if(!fn.startsWith(".csv")) fn += ".csv";
        m_ctx->currentWallet->history()->writeCSV(fn);
        Utils::showMessageBox("CSV export", QString("Transaction history exported to %1").arg(fn), false);
    });

    // Contact widget
    connect(ui->actionExportContactsCSV, &QAction::triggered, [=]{
        if(m_ctx->currentWallet == nullptr) return;
        auto *model = m_ctx->currentWallet->addressBookModel();
        if (model->rowCount() <= 0){
            Utils::showMessageBox("Error", "Addressbook empty", true);
            return;
        }

        const QString targetDir = QFileDialog::getExistingDirectory(this, "Select CSV output directory ", QDir::homePath(), QFileDialog::ShowDirsOnly);
        if(targetDir.isEmpty()) return;

        qint64 now = QDateTime::currentDateTime().currentMSecsSinceEpoch();
        QString fn = QString("%1/monero-contacts_%2.csv").arg(targetDir, QString::number(now / 1000));
        if(model->writeCSV(fn))
            Utils::showMessageBox("Address book exported", QString("Address book exported to %1").arg(fn), false);
    });

    // Tools
    connect(ui->actionSignVerify, &QAction::triggered, this, &MainWindow::menuSignVerifyClicked);
    connect(ui->actionVerifyTxProof, &QAction::triggered, this, &MainWindow::menuVerifyTxProof);

    // About screen
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::menuAboutClicked);
    connect(ui->actionDonate_to_Feather, &QAction::triggered, this, &MainWindow::donateButtonClicked);

    // Close / Open
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::menuWalletCloseClicked);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::menuWalletOpenClicked);
}

void MainWindow::menuToggleTabVisible(const QString &key){
    const auto toggleTab = m_tabShowHideMapper[key];
    bool show = config()->get(toggleTab->configKey).toBool();
    show = !show;
    config()->set(toggleTab->configKey, show);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
}

void MainWindow::initWidgets() {
    auto homeWidget = config()->get(Config::homeWidget).toString();
    if(homeWidget == QString("ccs")) {
        m_ccsWidget->show();
    } else if (homeWidget == "reddit") {
        m_redditWidget->show();
    } else {
        config()->set(Config::homeWidget, "ccs");
        m_ccsWidget->show();
    }
}

WalletWizard *MainWindow::createWizard(WalletWizard::Page startPage){
    auto *wizard = new WalletWizard(m_ctx, startPage, this);
    connect(wizard, &WalletWizard::openWallet, m_ctx, &AppContext::onOpenWallet);
    return wizard;
}

void MainWindow::showWizard(WalletWizard::Page startPage) {
    this->setEnabled(false);
    m_wizard = this->createWizard(startPage);
    m_wizard->show();
}

void MainWindow::onWalletClosed() {
    this->onWalletClosed(WalletWizard::Page_Menu);
}

void MainWindow::onWalletClosed(WalletWizard::Page page) {
    emit walletClosed();
    m_statusLabelBalance->clear();
    m_statusLabelStatus->clear();
    this->setWindowTitle("Feather");
    this->showWizard(page);
}

void MainWindow::onWalletCreatedError(const QString &err) {
    Utils::showMessageBox("Wallet creation error", err, true);
    this->showWizard(WalletWizard::Page_CreateWallet);
}

void MainWindow::onWalletOpenPasswordRequired(bool invalidPassword) {
    QInputDialog passwordDialog(this);
    passwordDialog.setInputMode(QInputDialog::TextInput);
    passwordDialog.setTextEchoMode(QLineEdit::Password);
    passwordDialog.setWindowTitle("Password required");
    passwordDialog.setLabelText("Please enter wallet password.");
    passwordDialog.resize(300, 100);
    if(!(bool)passwordDialog.exec())
        return this->showWizard(WalletWizard::Page_OpenWallet);

    const auto passwd = passwordDialog.textValue();
    m_ctx->walletPassword = passwd;
    m_ctx->onOpenWallet(m_ctx->walletPath, m_ctx->walletPassword);
}

void MainWindow::onWalletOpenedError(const QString &err) {
    qDebug() << Q_FUNC_INFO << QString("Wallet open error: %1").arg(err);
    QMessageBox::warning(this, "Wallet open error", err);
    this->setWindowTitle("Feather");
    this->showWizard(WalletWizard::Page_OpenWallet);
}

void MainWindow::onWalletCreated(Wallet *wallet) {
    qDebug() << Q_FUNC_INFO;
    // emit signal on behalf of walletManager
    m_ctx->walletManager->walletOpened(wallet);
}

void MainWindow::onWalletOpened() {
    qDebug() << Q_FUNC_INFO;
    if(m_wizard != nullptr) {
        m_wizard->hide();
        m_wizard->disconnect();
        m_wizard->deleteLater();
    }

    this->raise();
    this->show();
    this->activateWindow();
    this->setEnabled(true);
    if(!m_ctx->tor->torConnected)
        m_statusLabelStatus->setText("Wallet opened - Starting Tor (may take a while)");
    else
        m_statusLabelStatus->setText("Wallet opened - Searching for node");

    // window title as wallet name
    QFileInfo fileInfo(m_ctx->walletPath);
    this->setWindowTitle(QString("Feather - [%1]").arg(fileInfo.fileName()));

    connect(m_ctx->currentWallet, &Wallet::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);

    // receive page
    m_ctx->currentWallet->subaddress()->refresh( m_ctx->currentWallet->currentSubaddressAccount());
    ui->receiveWidget->setModel( m_ctx->currentWallet->subaddressModel(),  m_ctx->currentWallet->subaddress());
    if (m_ctx->currentWallet->subaddress()->count() == 1) {
        for (int i = 0; i < 10; i++) {
            m_ctx->currentWallet->subaddress()->addRow(m_ctx->currentWallet->currentSubaddressAccount(), "");
        }
    }
    connect(m_ctx->currentWallet->subaddress(), &Subaddress::labelChanged, [this]{
        m_ctx->storeWallet();
    });

    // history page
    m_ctx->currentWallet->history()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->historyWidget->setModel(m_ctx->currentWallet->coins(), m_ctx->currentWallet->historyModel(), m_ctx->currentWallet->history());
    connect(m_ctx->currentWallet->history(), &TransactionHistory::txNoteChanged, [this]{
        m_ctx->storeWallet();
    });

    // contacts widget
    ui->contactWidget->setModel(m_ctx->currentWallet->addressBookModel());
    connect(m_ctx->currentWallet->addressBook(), &AddressBook::descriptionChanged, [this]{
        m_ctx->storeWallet();
    });

    // coins page
    m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->coinsWidget->setModel(m_ctx->currentWallet->coinsModel(), m_ctx->currentWallet->coins());
    connect(m_ctx->currentWallet->coins(), &Coins::coinFrozen, [this]{
        m_ctx->storeWallet();
    });
    connect(m_ctx->currentWallet->coins(), &Coins::coinThawed, [this]{
        m_ctx->storeWallet();
    });
}

void MainWindow::onBalanceUpdated(double balance, double unlocked, const QString &balance_str, const QString &unlocked_str) {
    qDebug() << Q_FUNC_INFO;
    auto label_str = QString("Balance: %1 XMR").arg(unlocked_str);
    if(balance > unlocked)
        label_str += QString(" (+%1 XMR unconfirmed)").arg(QString::number(balance - unlocked, 'f'));
    m_statusLabelBalance->setText(label_str);
}

void MainWindow::onSynchronized() {
    m_statusLabelStatus->setText("Synchronized");
    this->onConnectionStatusChanged(Wallet::ConnectionStatus_Connected);
}

void MainWindow::onBlockchainSync(int height, int target) {
    QString heightText = QString("Blockchain sync: %1/%2").arg(height).arg(target);
    m_statusLabelStatus->setText(heightText);
}

void MainWindow::onRefreshSync(int height, int target) {
    QString heightText = QString("Wallet refresh: %1/%2").arg(height).arg(target);
    m_statusLabelStatus->setText(heightText);
}

void MainWindow::onConnectionStatusChanged(int status)
{
    qDebug() << "Wallet connection status changed " << status;

    // Update connection info in status bar.

    QString statusIcon;
    QString statusMsg;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            statusIcon = ":/assets/images/status_disconnected.svg";
            m_statusLabelStatus->setText("Disconnected");
            break;
        case Wallet::ConnectionStatus_Connected:
            if (m_ctx->currentWallet->synchronized()) {
                statusIcon = ":/assets/images/status_connected.svg";
            } else {
                statusIcon = ":/assets/images/status_waiting.svg";
            }
            break;
        case Wallet::ConnectionStatus_Connecting:
            statusIcon = ":/assets/images/status_lagging.svg";
            m_statusLabelStatus->setText("Connecting to daemon");
            break;
        case Wallet::ConnectionStatus_WrongVersion:
            statusIcon = ":/assets/images/status_disconnected.svg";
            m_statusLabelStatus->setText("Incompatible daemon");
            break;
        default:
            statusIcon = ":/assets/images/status_disconnected.svg";
            break;
    }

    m_statusBtnConnectionStatusIndicator->setIcon(QIcon(statusIcon));
}

void MainWindow::onCreateTransactionSuccess(PendingTransaction *tx, const QString &address, const quint32 &mixin) {
    auto tx_status = tx->status();
    auto err = QString("Can't create transaction: ");

    if(tx_status != PendingTransaction::Status_Ok){
        auto tx_err = tx->errorString();
        qCritical() << tx_err;

        if(m_ctx->currentWallet->connected() == Wallet::ConnectionStatus_WrongVersion)
            err = QString("%1 Wrong daemon version: %2").arg(err).arg(tx_err);
        else
            err = QString("%1 %2").arg(err).arg(tx_err);

        qDebug() << Q_FUNC_INFO << err;
        Utils::showMessageBox("Transaction error", err, true);
        m_ctx->currentWallet->disposeTransaction(tx);
    } else if (tx->txCount() == 0) {
        err = QString("%1 %2").arg(err).arg("No unmixable outputs to sweep.");
        qDebug() << Q_FUNC_INFO << err;
        Utils::showMessageBox("Transaction error", err, true);
        m_ctx->currentWallet->disposeTransaction(tx);
    } else {
        const auto &description = m_ctx->tmpTxDescription;

        auto *dialog = new TxConfDialog(tx, address, description, mixin, this);
        switch (dialog->exec()) {
            case QDialog::Rejected:
                m_ctx->onCancelTransaction(tx, address);
                break;
            case QDialog::Accepted:
                m_ctx->currentWallet->commitTransactionAsync(tx);
                break;
        }
    }
}

void MainWindow::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid) {
    if(status) { // success
        QString body = QString("Successfully sent %1 transaction(s).").arg(txid.count());
        QMessageBox::information(this, "Transactions sent", body);
        ui->sendWidget->clearFields();

        for(const auto &entry: txid) {
            m_ctx->currentWallet->setUserNote(entry, m_ctx->tmpTxDescription);
            AppContext::txDescriptionCache[entry] = m_ctx->tmpTxDescription;
        }

        m_ctx->tmpTxDescription = "";
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

void MainWindow::create_status_bar() {
    this->statusBar()->setFixedHeight(30);

    m_statusLabelStatus = new QLabel("Idle", this);
    m_statusLabelStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addWidget(m_statusLabelStatus);

    m_statusLabelBalance = new QLabel("Balance: 0.00 XMR", this);
    m_statusLabelBalance->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addPermanentWidget(m_statusLabelBalance);

    m_statusBtnConnectionStatusIndicator = new StatusBarButton(QIcon(":/assets/images/status_disconnected.svg"), "Connection status");
    connect(m_statusBtnConnectionStatusIndicator, &StatusBarButton::clicked, this, &MainWindow::showConnectionStatusDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnConnectionStatusIndicator);

    m_statusBtnPassword = new StatusBarButton(QIcon(":/assets/images/lock.svg"), "Password");
    connect(m_statusBtnPassword, &StatusBarButton::clicked, this, &MainWindow::showPasswordDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnPassword);

    m_statusBtnPreferences = new StatusBarButton(QIcon(":/assets/images/preferences.svg"), "Settings");
    connect(m_statusBtnPreferences, &StatusBarButton::clicked, this, &MainWindow::menuSettingsClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnPreferences);

    m_statusBtnSeed = new StatusBarButton(QIcon(":/assets/images/seed.png"), "Seed");
    connect(m_statusBtnSeed, &StatusBarButton::clicked, this, &MainWindow::showSeedDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnSeed);

    m_statusBtnTor = new StatusBarButton(QIcon(":/assets/images/tor_logo_disabled.png"), "Tor");
    connect(m_statusBtnTor, &StatusBarButton::clicked, this, &MainWindow::menuTorClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnTor);
}

void MainWindow::showWalletInfoDialog() {
    auto *dialog = new WalletInfoDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showSeedDialog() {
    QString seed = m_ctx->currentWallet->getCacheAttribute("feather.seed");
    if (seed.isEmpty()) {
        seed = m_ctx->currentWallet->getSeed();
    }

    auto *dialog = new SeedDialog(seed, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showConnectionStatusDialog() {
    auto status = m_ctx->currentWallet->connected(true);
    bool synchronized = m_ctx->currentWallet->synchronized();

    QString statusMsg;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            statusMsg = "Wallet is disconnected from daemon.";
            break;
        case Wallet::ConnectionStatus_Connected: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connected to %1").arg(node.full);
            if (synchronized)
                statusMsg += " and synchronized";
            break;
        }
        case Wallet::ConnectionStatus_Connecting: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connecting to %1").arg(node.full);
            break;
        }
        case Wallet::ConnectionStatus_WrongVersion:
            statusMsg = "Wallet is connected to incompatible daemon.";
            break;
        default:
            statusMsg = "Unknown connection status (this should never happen).";
    }

    QMessageBox::information(this, "Connection Status", statusMsg);
}

void MainWindow::showPasswordDialog() {
    auto *pdialog = new PasswordChangeDialog(this);
    int ret = pdialog->exec();
    if (!ret) return;

    QApplication::setActiveWindow(this);

    QString currentPassword = pdialog->getCurrentPassword();
    QString newPassword = pdialog->getNewPassword();

    if (currentPassword != m_ctx->walletPassword) {
        QMessageBox::warning(this, "Error", "Incorrect password");
        return;
    }

    if (m_ctx->currentWallet->setPassword(newPassword)) {
        QMessageBox::information(this, "Information", "Password changed successfully");
    }
    else {
        QMessageBox::warning(this, "Error", QString("Error: %1").arg(m_ctx->currentWallet->errorString()));
    }

    pdialog->deleteLater();
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

void MainWindow::menuTorClicked() {

    auto *dialog = new TorInfoDialog(m_ctx, this);
    connect(m_ctx->tor, &Tor::logsUpdated, dialog, &TorInfoDialog::onLogsUpdated);
    dialog->exec();
    disconnect(m_ctx->tor, &Tor::logsUpdated, dialog, &TorInfoDialog::onLogsUpdated);
    dialog->deleteLater();
}

void MainWindow::menuNewRestoreClicked() {
    // TODO: implement later
}

void MainWindow::menuQuitClicked() {
    QCoreApplication::quit();
}

void MainWindow::menuWalletCloseClicked() {
    if(m_ctx->currentWallet == nullptr) return;
    m_ctx->walletClose(true);
}

void MainWindow::menuWalletOpenClicked() {
    auto walletPath = config()->get(Config::walletPath).toString();
    if(walletPath.isEmpty())
        walletPath = m_ctx->defaultWalletDir;
    QString path = QFileDialog::getOpenFileName(this, "Select your wallet file", walletPath, "Wallet file (*.keys)");
    if(path.isEmpty()) return;

    QFileInfo infoPath(path);
    if(!infoPath.isReadable()) {
        Utils::showMessageBox("Cannot read wallet file", "Permission error.", true);
        return;
    }

    if(path == m_ctx->walletPath) {
        Utils::showMessageBox("Wallet already opened", "Please open another wallet.", true);
        return;
    }

    m_ctx->walletClose(false);
    emit walletClosed();
    m_ctx->onOpenWallet(path, "");
}

void MainWindow::menuAboutClicked() {
    m_aboutDialog->show();
}

void MainWindow::menuSettingsClicked() {
    m_windowSettings->show();
}

void MainWindow::menuSignVerifyClicked() {
    auto *dialog = new SignVerifyDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::menuVerifyTxProof() {
    auto *dialog = new VerifyProofDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::skinChanged(const QString &skinName) {
    if(!m_skins.contains(skinName)) {
        qWarning() << QString("No such skin %1").arg(skinName);
        return;
    }

    config()->set(Config::skin, skinName);
    qApp->setStyleSheet(m_skins[skinName]);
    qDebug() << QString("Skin changed to %1").arg(skinName);
}

void MainWindow::homeWidgetChanged(const QString &widgetName) {
    if(widgetName == "ccs"){
        m_ccsWidget->show();
        m_redditWidget->hide();
    } else if(widgetName == "reddit") {
        m_ccsWidget->hide();
        m_redditWidget->show();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_ctx->tor->stop();

    this->saveGeo();
    if(m_wizard != nullptr)
        m_wizard->close();

    QWidget::closeEvent(event);
}

void MainWindow::donateButtonClicked() {
    double donation = AppContext::prices->convert("EUR", "XMR", m_ctx->featherDonationAmount);
    if(donation <= 0)
        donation = 1.337;

    ui->sendWidget->fill(m_ctx->featherDonationAddress, "Donation to the Feather development team", donation);
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::showHistoryTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::showSendTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::showCalcWindow() {
    m_windowCalc->show();
}

void MainWindow::showSendScreen(const CCSEntry &entry) {
    ui->sendWidget->fill(entry);
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::onViewOnBlockExplorer(const QString &txid) {
    QString blockExplorerLink = Utils::blockExplorerLink(config()->get(Config::blockExplorer).toString(), m_ctx->networkType, txid);
    Utils::externalLinkWarning(blockExplorerLink);
}

void MainWindow::onAddContact(const QString &address, const QString &name) {
    bool addressValid = WalletManager::addressValid(address, m_ctx->currentWallet->nettype());
    if (!addressValid)
        QMessageBox::warning(this, "Invalid address", "Invalid address");
    else {
        m_ctx->currentWallet->addressBook()->addRow(address, "", name);
        m_ctx->storeWallet();
    }
}

MainWindow *MainWindow::getInstance() {
    return pMainWindow;
}

AppContext *MainWindow::getContext(){
    return pMainWindow->m_ctx;
}

void MainWindow::loadSkins() {
    QString qdarkstyle = this->loadStylesheet(":qdarkstyle/style.qss");
    if (!qdarkstyle.isEmpty())
        m_skins.insert("QDarkStyle", qdarkstyle);

    QString breeze_dark = this->loadStylesheet(":/dark.qss");
    if (!breeze_dark.isEmpty())
        m_skins.insert("Breeze/Dark", breeze_dark);

    QString breeze_light = this->loadStylesheet(":/light.qss");
    if (!breeze_light.isEmpty())
        m_skins.insert("Breeze/Light", breeze_light);
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

void MainWindow::showNodeExhaustedMessage() {
    // Spawning dialogs inside a lambda can cause system freezes on linux so we have to do it this way ¯\_(ツ)_/¯

    auto msg = "Feather is in 'custom node connection mode' but could not "
               "find an eligible node to connect to. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

void MainWindow::showWSNodeExhaustedMessage() {
    auto msg = "Feather is in 'automatic node connection mode' but the "
               "websocket server returned none available nodes. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

MainWindow::~MainWindow() {
    delete ui;
}
