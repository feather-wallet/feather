// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QDir>
#include <QMessageBox>

#include "appcontext.h"
#include "globals.h"

// libwalletqt
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/Subaddress.h"
#include "libwalletqt/Coins.h"
#include "model/TransactionHistoryModel.h"
#include "model/SubaddressModel.h"
#include "utils/NetworkManager.h"
#include "utils/WebsocketClient.h"
#include "utils/WebsocketNotifier.h"

WalletKeysFilesModel *AppContext::wallets = nullptr;
QMap<QString, QString> AppContext::txCache;

AppContext::AppContext(QCommandLineParser *cmdargs) {
    this->cmdargs = cmdargs;

    this->isTails = TailsOS::detect();
    this->isWhonix = WhonixOS::detect();

    // ----------------- Setup Paths -----------------

    QString configDir = Config::defaultConfigDir().path();
    createConfigDirectory(configDir);

    QString walletDir = config()->get(Config::walletDirectory).toString();
    if (walletDir.isEmpty()) {
        walletDir = Utils::defaultWalletDir();
    }
    this->defaultWalletDir = walletDir;
    if (!QDir().mkpath(defaultWalletDir))
        qCritical() << "Unable to create dir: " << defaultWalletDir;

    // ----------------- Network Type -----------------

    if (this->cmdargs->isSet("stagenet")) {
        this->networkType = NetworkType::STAGENET;
        config()->set(Config::networkType, NetworkType::STAGENET);
    }
    else if (this->cmdargs->isSet("testnet")) {
        this->networkType = NetworkType::TESTNET;
        config()->set(Config::networkType, NetworkType::TESTNET);
    }
    else {
        this->networkType = NetworkType::MAINNET;
        config()->set(Config::networkType, NetworkType::MAINNET);
    }

    this->nodes = new Nodes(this, this);

    // Store the wallet every 2 minutes
    m_storeTimer.start(2 * 60 * 1000);
    connect(&m_storeTimer, &QTimer::timeout, [this](){
        this->storeWallet();
    });

    this->walletManager = WalletManager::instance();
    QString logPath = QString("%1/daemon.log").arg(configDir);
    Monero::Utils::onStartup();
    Monero::Wallet::init("", "feather", logPath.toStdString(), true);

    bool logLevelFromEnv;
    int logLevel = qEnvironmentVariableIntValue("MONERO_LOG_LEVEL", &logLevelFromEnv);
    if (this->cmdargs->isSet("quiet"))
        this->walletManager->setLogLevel(-1);
    else if (logLevelFromEnv && logLevel >= 0 && logLevel <= Monero::WalletManagerFactory::LogLevel_Max)
        Monero::WalletManagerFactory::setLogLevel(logLevel);

    connect(this, &AppContext::createTransactionError, this, &AppContext::onCreateTransactionError);

    // libwallet connects
    connect(this->walletManager, &WalletManager::walletOpened, this, &AppContext::onWalletOpened);
    connect(this->walletManager, &WalletManager::walletCreated, this, &AppContext::onWalletCreated);
    connect(this->walletManager, &WalletManager::deviceButtonRequest, this, &AppContext::onDeviceButtonRequest);
    connect(this->walletManager, &WalletManager::deviceError, this, &AppContext::onDeviceError);

    // TODO: move me
    connect(websocketNotifier(), &WebsocketNotifier::NodesReceived, this->nodes, &Nodes::onWSNodesReceived);

    m_rpc = new DaemonRpc{this, getNetworkTor(), ""};
}

void AppContext::initTor() {
    if (this->cmdargs->isSet("tor-host"))
        config()->set(Config::socks5Host, this->cmdargs->value("tor-host"));
    if (this->cmdargs->isSet("tor-port"))
        config()->set(Config::socks5Port, this->cmdargs->value("tor-port"));
    if (this->cmdargs->isSet("use-local-tor"))
        config()->set(Config::useLocalTor, true);

    torManager()->init();
    torManager()->start();

    connect(torManager(), &TorManager::connectionStateChanged, &websocketNotifier()->websocketClient, &WebsocketClient::onToggleConnect);

    this->onTorSettingsChanged();
}

void AppContext::initWS() {
    websocketNotifier()->websocketClient.start();
}

void AppContext::onCancelTransaction(PendingTransaction *tx, const QVector<QString> &address) {
    // tx cancelled by user
    double amount = tx->amount() / globals::cdiv;
    emit createTransactionCancelled(address, amount);
    this->currentWallet->disposeTransaction(tx);
}

void AppContext::onSweepOutput(const QString &keyImage, QString address, bool churn, int outputs) {
    if(this->currentWallet == nullptr){
        qCritical() << "Cannot create transaction; no wallet loaded";
        return;
    }

    if (churn) {
        address = this->currentWallet->address(0, 0); // primary address
    }

    qCritical() << "Creating transaction";
    this->currentWallet->createTransactionSingleAsync(keyImage, address, outputs, this->tx_priority);

    emit initiateTransaction();
}

void AppContext::onCreateTransaction(const QString &address, quint64 amount, const QString &description, bool all) {
    // tx creation
    this->tmpTxDescription = description;

    if(this->currentWallet == nullptr) {
        emit createTransactionError("Cannot create transaction; no wallet loaded");
        return;
    }

    if (!all && amount == 0) {
        emit createTransactionError("Cannot send nothing");
        return;
    }

    auto unlocked_balance = this->currentWallet->unlockedBalance();
    if(!all && amount > unlocked_balance) {
        emit createTransactionError("Not enough money to spend");
        return;
    } else if(unlocked_balance == 0) {
        emit createTransactionError("No money to spend");
        return;
    }

    qDebug() << "creating tx";
    if (all)
        this->currentWallet->createTransactionAllAsync(address, "", globals::mixin, this->tx_priority);
    else
        this->currentWallet->createTransactionAsync(address, "", amount, globals::mixin, this->tx_priority);

    emit initiateTransaction();
}

void AppContext::onCreateTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description) {
    this->tmpTxDescription = description;

    if (this->currentWallet == nullptr) {
        emit createTransactionError("Cannot create transaction; no wallet loaded");
        return;
    }

    quint64 total_amount = 0;
    for (auto &amount : amounts) {
        total_amount += amount;
    }

    auto unlocked_balance = this->currentWallet->unlockedBalance();
    if (total_amount > unlocked_balance) {
        emit createTransactionError("Not enough money to spend");
    }

    qDebug() << "Creating tx";
    this->currentWallet->createTransactionMultiDestAsync(addresses, amounts, this->tx_priority);

    emit initiateTransaction();
}

void AppContext::onCreateTransactionError(const QString &msg) {
    this->tmpTxDescription = "";
    emit endTransaction();
}

void AppContext::closeWallet(bool emitClosedSignal, bool storeWallet) {
    if (this->currentWallet == nullptr)
        return;

    emit walletAboutToClose();

    if (storeWallet) {
        this->storeWallet();
    }

    this->currentWallet->disconnect();
    this->walletManager->closeWallet();
    this->currentWallet = nullptr;

    if (emitClosedSignal)
        emit walletClosed();
}

void AppContext::onOpenWallet(const QString &path, const QString &password){
    if(this->currentWallet != nullptr){
        emit walletOpenedError("There is an active wallet opened.");
        return;
    }

    if(!Utils::fileExists(path)) {
        emit walletOpenedError(QString("Wallet not found: %1").arg(path));
        return;
    }

    if (password.isEmpty()) {
        this->walletPassword = "";
    }

    config()->set(Config::firstRun, false);

    this->walletPath = path;
    this->walletManager->openWalletAsync(path, password, this->networkType, 1);
}

void AppContext::onWalletCreated(Wallet * wallet) {
    // Currently only called when a wallet is created from device.
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        emit walletCreatedError(wallet->errorString());
        return;
    }

    this->onWalletOpened(wallet);
}

void AppContext::onPreferredFiatCurrencyChanged(const QString &symbol) {
    if(this->currentWallet) {
        auto *model = this->currentWallet->transactionHistoryModel();
        if(model != nullptr) {
            model->preferredFiatSymbol = symbol;
        }
    }
}

void AppContext::onAmountPrecisionChanged(int precision) {
    if (!this->currentWallet) return;
    auto *model = this->currentWallet->transactionHistoryModel();
    if (!model) return;
    model->amountPrecision = precision;
}

void AppContext::commitTransaction(PendingTransaction *tx) {
    // Nodes - even well-connected, properly configured ones - consistently fail to relay transactions
    // To mitigate transactions failing we just send the transaction to every node we know about over Tor
    if (config()->get(Config::multiBroadcast).toBool()) {
        this->onMultiBroadcast(tx);
    }

    this->currentWallet->commitTransactionAsync(tx);
}

void AppContext::onMultiBroadcast(PendingTransaction *tx) {
    int count = tx->txCount();
    for (int i = 0; i < count; i++) {
        QString txData = tx->signedTxToHex(i);

        for (const auto& node: this->nodes->websocketNodes()) {
            if (!node.online) continue;

            QString address = node.toURL();
            qDebug() << QString("Relaying %1 to: %2").arg(tx->txid()[i], address);
            m_rpc->setDaemonAddress(address);
            m_rpc->sendRawTransaction(txData);
        }
    }
}

void AppContext::onDeviceButtonRequest(quint64 code) {
    emit deviceButtonRequest(code);
}

void AppContext::onDeviceError(const QString &message) {
    qCritical() << "Device error: " << message;
    emit deviceError(message);
}

void AppContext::onTorSettingsChanged() {
    if (WhonixOS::detect() || Utils::isTorsocks()) {
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

    this->nodes->connectToNode();

    auto privacyLevel = config()->get(Config::torPrivacyLevel).toInt();
    qDebug() << "Changed privacyLevel to " << privacyLevel;
}

void AppContext::onInitialNetworkConfigured() {
    this->initTor();
    this->initWS();
}

void AppContext::onWalletOpened(Wallet *wallet) {
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        auto errMsg = wallet->errorString();
        if (state == Wallet::Status_BadPassword) {
            this->closeWallet(false);
            // Don't show incorrect password when we try with empty password for the first time
            bool showIncorrectPassword = m_openWalletTriedOnce;
            m_openWalletTriedOnce = true;
            emit walletOpenPasswordNeeded(showIncorrectPassword, wallet->path());
        }
        else if (errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc")) {
            qCritical() << errMsg;
            this->walletManager->clearWalletCache(this->walletPath);
            errMsg = QString("%1\n\nAttempted to clean wallet cache. Please restart Feather.").arg(errMsg);
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else {
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        }

        return;
    }

    m_openWalletTriedOnce = false;
    this->refreshed = false;
    this->currentWallet = wallet;
    this->walletPath = this->currentWallet->path() + ".keys";
    this->walletPassword = this->currentWallet->getPassword();
    config()->set(Config::walletPath, this->walletPath);

    connect(this->currentWallet, &Wallet::moneySpent, this, &AppContext::onMoneySpent);
    connect(this->currentWallet, &Wallet::moneyReceived, this, &AppContext::onMoneyReceived);
    connect(this->currentWallet, &Wallet::unconfirmedMoneyReceived, this, &AppContext::onUnconfirmedMoneyReceived);
    connect(this->currentWallet, &Wallet::newBlock, this, &AppContext::onWalletNewBlock);
    connect(this->currentWallet, &Wallet::updated, this, &AppContext::onWalletUpdate);
    connect(this->currentWallet, &Wallet::refreshed, this, &AppContext::onWalletRefreshed);
    connect(this->currentWallet, &Wallet::transactionCommitted, this, &AppContext::onTransactionCommitted);
    connect(this->currentWallet, &Wallet::heightRefreshed, this, &AppContext::onHeightRefreshed);
    connect(this->currentWallet, &Wallet::transactionCreated, this, &AppContext::onTransactionCreated);
    connect(this->currentWallet, &Wallet::deviceError, this, &AppContext::onDeviceError);
    connect(this->currentWallet, &Wallet::deviceButtonRequest, this, &AppContext::onDeviceButtonRequest);

    emit walletOpened();

    connect(this->currentWallet, &Wallet::connectionStatusChanged, [this]{
        this->nodes->autoConnect();
    });
    this->nodes->connectToNode();
    this->updateBalance();

#ifdef DONATE_BEG
    this->donateBeg();
#endif

    // force trigger preferredFiat signal for history model
    this->onPreferredFiatCurrencyChanged(config()->get(Config::preferredFiatCurrency).toString());
}

void AppContext::createConfigDirectory(const QString &dir) {
    QString config_dir_tor = QString("%1/%2").arg(dir).arg("tor");
    QString config_dir_tordata = QString("%1/%2").arg(dir).arg("tor/data");

    QStringList createDirs({dir, config_dir_tor, config_dir_tordata});
    for(const auto &d: createDirs) {
        if(!Utils::dirExists(d)) {
            qDebug() << QString("Creating directory: %1").arg(d);
            if (!QDir().mkpath(d)) {
                qCritical() << "Could not create directory " << d;
            }
        }
    }
}

void AppContext::createWallet(FeatherSeed seed, const QString &path, const QString &password, const QString &seedOffset) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(seed.mnemonic.isEmpty()) {
        emit walletCreatedError("Mnemonic seed error. Failed to write wallet.");
        return;
    }

    Wallet *wallet = nullptr;
    if (seed.seedType == SeedType::TEVADOR) {
        wallet = this->walletManager->createDeterministicWalletFromSpendKey(path, password, seed.language, this->networkType, seed.spendKey, seed.restoreHeight, globals::kdfRounds, seedOffset);
        wallet->setCacheAttribute("feather.seed", seed.mnemonic.join(" "));
        wallet->setCacheAttribute("feather.seedoffset", seedOffset);
    }
    if (seed.seedType == SeedType::MONERO) {
        wallet = this->walletManager->recoveryWallet(path, password, seed.mnemonic.join(" "), seedOffset, this->networkType, seed.restoreHeight, globals::kdfRounds);
    }

    this->currentWallet = wallet;
    if(this->currentWallet == nullptr) {
        emit walletCreatedError("Failed to write wallet");
        return;
    }

    this->onWalletOpened(wallet);
}

void AppContext::createWalletFromDevice(const QString &path, const QString &password, int restoreHeight) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    this->walletManager->createWalletFromDeviceAsync(path, password, this->networkType, "Ledger", restoreHeight);
}

void AppContext::createWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, bool deterministic) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!WalletManager::addressValid(address, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid address provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!this->walletManager->keyValid(viewkey, address, true, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid viewkey provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!spendkey.isEmpty() && !this->walletManager->keyValid(spendkey, address, false, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid spendkey provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    Wallet *wallet = this->walletManager->createWalletFromKeys(path, password, this->seedLanguage, this->networkType, address, viewkey, spendkey, restoreHeight);
    this->walletManager->walletOpened(wallet);
}

void AppContext::onSetRestoreHeight(quint64 height){
    auto seed = this->currentWallet->getCacheAttribute("feather.seed");
    if(!seed.isEmpty()) {
        const auto msg = "This wallet has a 14 word mnemonic seed which has the restore height embedded.";
        emit setRestoreHeightError(msg);
        return;
    }

    this->currentWallet->setWalletCreationHeight(height);
    this->currentWallet->setPassword(this->currentWallet->getPassword());  // trigger .keys write

    // nuke wallet cache
    const auto fn = this->currentWallet->path();
    this->walletManager->clearWalletCache(fn);

    emit customRestoreHeightSet(height);
}

void AppContext::onOpenAliasResolve(const QString &openAlias) {
    // @TODO: calling this freezes for about 1-2 seconds :/
    const auto result = this->walletManager->resolveOpenAlias(openAlias);
    const auto spl = result.split("|");
    auto msg = QString("");
    if(spl.count() != 2) {
        msg = "Internal error";
        emit openAliasResolveError(msg);
        return;
    }

    const auto &status = spl.at(0);
    const auto &address = spl.at(1);
    const auto valid = this->walletManager->addressValid(address, this->networkType);
    if(status == "false"){
        if(valid){
            msg = "Address found, but the DNSSEC signatures could not be verified, so this address may be spoofed";
            emit openAliasResolveError(msg);
            return;
        } else {
            msg = "No valid address found at this OpenAlias address, but the DNSSEC signatures could not be verified, so this may be spoofed";
            emit openAliasResolveError(msg);
            return;
        }
    } else if(status != "true") {
        msg = "Internal error";
        emit openAliasResolveError(msg);
        return;
    }

    if(valid){
        emit openAliasResolved(address, openAlias);
        return;
    }

    msg = QString("Address validation error.");
    if(!address.isEmpty())
        msg += QString(" Perhaps it is of the wrong network type."
                       "\n\nOpenAlias: %1\nAddress: %2").arg(openAlias).arg(address);
    emit openAliasResolveError(msg);
}

void AppContext::donateBeg() {
    if (this->currentWallet == nullptr) return;
    if (this->networkType != NetworkType::Type::MAINNET) return;
    if (this->currentWallet->viewOnly()) return;

    auto donationCounter = config()->get(Config::donateBeg).toInt();
    if (donationCounter == -1)
        return;  // previously donated

    donationCounter += 1;
    if (donationCounter % globals::donationBoundary == 0) {
        emit donationNag();
    }
    config()->set(Config::donateBeg, donationCounter);
}

AppContext::~AppContext() = default;

// ############################################## LIBWALLET QT #########################################################

void AppContext::onMoneySpent(const QString &txId, quint64 amount) {
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);
}

void AppContext::onMoneyReceived(const QString &txId, quint64 amount) {
    // Incoming tx included in a block.
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);
}

void AppContext::onUnconfirmedMoneyReceived(const QString &txId, quint64 amount) {
    // Incoming transaction in pool
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);

    if(this->currentWallet->synchronized()) {
        auto notify = QString("%1 XMR (pending)").arg(amount_num);
        Utils::desktopNotify("Payment received", notify, 5000);
    }
}

void AppContext::onWalletUpdate() {
    if (this->currentWallet->synchronized()) {
        this->refreshModels();
        this->storeWallet();
    }

    this->updateBalance();
}

void AppContext::onWalletRefreshed(bool success, const QString &message) {
    if (!success) {
        // Something went wrong during refresh, in some cases we need to notify the user
        qCritical() << "Exception during refresh: " << message; // Can't use ->errorString() here, other SLOT might snipe it first
        return;
    }

    if (!this->refreshed) {
        refreshModels();
        this->refreshed = true;
        emit walletRefreshed();
        // store wallet immediately upon finishing synchronization
        this->currentWallet->store();
    }

    qDebug() << "Wallet refresh status: " << success;
}

void AppContext::onWalletNewBlock(quint64 blockheight, quint64 targetHeight) {
    // Called whenever a new block gets scanned by the wallet
    this->syncStatusUpdated(blockheight, targetHeight);

    if (!this->currentWallet) return;
    if (this->currentWallet->isSynchronized()) {
        this->currentWallet->coins()->refreshUnlocked();
        this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
        // Todo: only refresh tx confirmations
    }
}

void AppContext::onHeightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight) {
    qDebug() << Q_FUNC_INFO << walletHeight << daemonHeight << targetHeight;

    if (this->currentWallet->connectionStatus() == Wallet::ConnectionStatus_Disconnected)
        return;

    if (daemonHeight < targetHeight) {
        emit blockchainSync(daemonHeight, targetHeight);
    }
    else {
        this->syncStatusUpdated(walletHeight, daemonHeight);
    }
}

void AppContext::onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address) {
    qDebug() << Q_FUNC_INFO;

    for (auto &addr : address) {
        if (addr == globals::donationAddress) {
            this->donationSending = true;
        }
    }

    // Let UI know that the transaction was constructed
    emit endTransaction();

    // tx created, but not sent yet. ask user to verify first.
    emit createTransactionSuccess(tx, address);
}

void AppContext::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid){
    if (status) {
        for (const auto &entry: txid) {
            this->currentWallet->setUserNote(entry, this->tmpTxDescription);
        }
        this->tmpTxDescription = "";
    }

    // Store wallet immediately so we don't risk losing tx key if wallet crashes
    this->currentWallet->store();

    this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->coins()->refresh(this->currentWallet->currentSubaddressAccount());

    this->updateBalance();

    // this tx was a donation to Feather, stop our nagging
    if(this->donationSending) {
        this->donationSending = false;
        config()->set(Config::donateBeg, -1);
    }

    emit transactionCommitted(status, tx, txid);
}

void AppContext::storeWallet() {
    // Do not store a synchronizing wallet: store() is NOT thread safe and may crash the wallet
    if (this->currentWallet == nullptr || !this->currentWallet->isSynchronized())
        return;

    qDebug() << "Storing wallet";
    this->currentWallet->store();
}

void AppContext::updateBalance() {
    if (!this->currentWallet)
        return;

    quint64 balance = this->currentWallet->balance();
    quint64 spendable = this->currentWallet->unlockedBalance();

    emit balanceUpdated(balance, spendable);
}

void AppContext::syncStatusUpdated(quint64 height, quint64 target) {
    if (height < (target - 1)) {
        emit refreshSync(height, target);
    }
    else {
        this->updateBalance();
        emit synchronized();
    }
}

void AppContext::refreshModels() {
    if (!this->currentWallet)
        return;

    this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->subaddress()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->coins()->refresh(this->currentWallet->currentSubaddressAccount());
    // Todo: set timer for refreshes
}
