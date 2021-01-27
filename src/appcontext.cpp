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


Prices *AppContext::prices = nullptr;
WalletKeysFilesModel *AppContext::wallets = nullptr;
TxFiatHistory *AppContext::txFiatHistory = nullptr;
double AppContext::balance = 0;
QMap<QString, QString> AppContext::txDescriptionCache;
QMap<QString, QString> AppContext::txCache;

AppContext::AppContext(QCommandLineParser *cmdargs) {
    this->network = new QNetworkAccessManager();
    this->networkClearnet = new QNetworkAccessManager();
    this->cmdargs = cmdargs;

#if defined(Q_OS_MAC)
    this->isTorSocks = qgetenv("DYLD_INSERT_LIBRARIES").indexOf("libtorsocks") >= 0;
#elif defined(Q_OS_LINUX)
    this->isTorSocks = qgetenv("LD_PRELOAD").indexOf("libtorsocks") >= 0;
#elif defined(Q_OS_WIN)
    this->isTorSocks = false;
#endif

    this->isTails = TailsOS::detect();
    this->isWhonix = WhonixOS::detect();

    //Paths
    this->configRoot = QDir::homePath();
    if (isTails) { // #if defined(PORTABLE)
        QString portablePath = []{
            QString appImagePath = qgetenv("APPIMAGE");
            if (appImagePath.isEmpty()) {
                qDebug() << "Not an appimage, using currentPath()";
                return QDir::currentPath() + "/.feather";
            }

            QFileInfo appImageDir(appImagePath);
            return appImageDir.absoluteDir().path() + "/.feather";
        }();


        if (QDir().mkpath(portablePath)) {
            this->configRoot = portablePath;
        } else {
            qCritical() << "Unable to create portable directory: " << portablePath;
        }
    }

    this->accountName = Utils::getUnixAccountName();
    this->homeDir = QDir::homePath();

    QString walletDir = config()->get(Config::walletDirectory).toString();
    if (walletDir.isEmpty()) {
#if defined(Q_OS_LINUX) or defined(Q_OS_MAC)
        this->defaultWalletDir = QString("%1/Monero/wallets").arg(this->configRoot);
        this->defaultWalletDirRoot = QString("%1/Monero").arg(this->configRoot);
#elif defined(Q_OS_WIN)
        this->defaultWalletDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Monero";
        this->defaultWalletDirRoot = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    } else {
        this->defaultWalletDir = walletDir;
        this->defaultWalletDirRoot = walletDir;
    }

    // Create wallet dirs
    if (!QDir().mkpath(defaultWalletDir))
        qCritical() << "Unable to create dir: " << defaultWalletDir;

    this->configDirectory = QString("%1/.config/feather/").arg(this->configRoot);
#if defined(Q_OS_UNIX)
    if(!this->configDirectory.endsWith('/'))
        this->configDirectory = QString("%1/").arg(this->configDirectory);
#endif

    // Config
    createConfigDirectory(this->configDirectory);

    if(this->cmdargs->isSet("stagenet"))
        this->networkType = NetworkType::STAGENET;
    else if(this->cmdargs->isSet("testnet"))
        this->networkType = NetworkType::TESTNET;
    else
        this->networkType = NetworkType::MAINNET;

//    auto nodeSourceUInt = config()->get(Config::nodeSource).toUInt();
//    AppContext::nodeSource = static_cast<NodeSource>(nodeSourceUInt);
    this->nodes = new Nodes(this, this->networkClearnet);
    connect(this, &AppContext::nodeSourceChanged, this->nodes, &Nodes::onNodeSourceChanged);
    connect(this, &AppContext::setCustomNodes, this->nodes, &Nodes::setCustomNodes);

    // Tor & socks proxy
    this->ws = new WSClient(this, m_wsUrl);
    connect(this->ws, &WSClient::WSMessage, this, &AppContext::onWSMessage);

    // Store the wallet every 2 minutes
    m_storeTimer.start(2 * 60 * 1000);
    connect(&m_storeTimer, &QTimer::timeout, [this](){
        this->storeWallet();
    });

    // restore height lookup
    this->initRestoreHeights();

    // price history lookup
    auto genesis_timestamp = this->restoreHeights[NetworkType::Type::MAINNET]->data.firstKey();
    AppContext::txFiatHistory = new TxFiatHistory(genesis_timestamp, this->configDirectory);
    connect(this->ws, &WSClient::connectionEstablished, AppContext::txFiatHistory, &TxFiatHistory::onUpdateDatabase);
    connect(AppContext::txFiatHistory, &TxFiatHistory::requestYear, [=](int year){
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1}})").arg(year).toUtf8();
        this->ws->sendMsg(data);
    });
    connect(AppContext::txFiatHistory, &TxFiatHistory::requestYearMonth, [=](int year, int month) {
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1, "month": %2}})").arg(year).arg(month).toUtf8();
        this->ws->sendMsg(data);
    });

    // fiat/crypto lookup
    AppContext::prices = new Prices();

    // xmr.to
#ifdef HAS_XMRTO
    this->XMRTo = new XmrTo(this);
#endif

    // XMRig
#ifdef HAS_XMRIG
    this->XMRig = new XmRig(this->configDirectory, this);
    this->XMRig->prepare();
#endif

    this->walletManager = WalletManager::instance();
    QString logPath = QString("%1/daemon.log").arg(configDirectory);
    Monero::Utils::onStartup();
    Monero::Wallet::init("", "feather", logPath.toStdString(), true);

    bool logLevelFromEnv;
    int logLevel = qEnvironmentVariableIntValue("MONERO_LOG_LEVEL", &logLevelFromEnv);
    if(this->cmdargs->isSet("quiet"))
        this->walletManager->setLogLevel(-1);
    else if (logLevelFromEnv && logLevel >= 0 && logLevel <= Monero::WalletManagerFactory::LogLevel_Max)
        Monero::WalletManagerFactory::setLogLevel(logLevel);

    connect(this, &AppContext::createTransactionError, this, &AppContext::onCreateTransactionError);

    // libwallet connects
    connect(this->walletManager, &WalletManager::walletOpened, this, &AppContext::onWalletOpened);
}

void AppContext::initTor() {
    this->tor = new Tor(this, this);
    this->tor->start();

    if (!(isWhonix)) {
        this->networkProxy = new QNetworkProxy(QNetworkProxy::Socks5Proxy, Tor::torHost, Tor::torPort);
        this->network->setProxy(*networkProxy);
        if (m_wsUrl.host().endsWith(".onion"))
            this->ws->webSocket.setProxy(*networkProxy);
    }

}

void AppContext::initWS() {
    this->ws->start();
}

void AppContext::onCancelTransaction(PendingTransaction *tx, const QVector<QString> &address) {
    // tx cancelled by user
    double amount = tx->amount() / globals::cdiv;
    emit createTransactionCancelled(address, amount);
    this->currentWallet->disposeTransaction(tx);
}

void AppContext::onSweepOutput(const QString &keyImage, QString address, bool churn, int outputs) const {
    if(this->currentWallet == nullptr){
        qCritical() << "Cannot create transaction; no wallet loaded";
        return;
    }

    if (churn) {
        address = this->currentWallet->address(0, 0); // primary address
    }

    qCritical() << "Creating transaction";
    this->currentWallet->createTransactionSingleAsync(keyImage, address, outputs, this->tx_priority);
}

void AppContext::onCreateTransaction(XmrToOrder *order) {
    // tx creation via xmr.to
    const QString description = QString("XmrTo order %1").arg(order->uuid);
    quint64 amount = WalletManager::amountFromDouble(order->incoming_amount_total);
    this->onCreateTransaction(order->receiving_subaddress, amount, description, false);
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

    auto balance = this->currentWallet->balance();
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
        this->currentWallet->createTransactionAllAsync(address, "", this->tx_mixin, this->tx_priority);
    else
        this->currentWallet->createTransactionAsync(address, "", amount, this->tx_mixin, this->tx_priority);

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

void AppContext::onPreferredFiatCurrencyChanged(const QString &symbol) {
    if(this->currentWallet) {
        auto *model = this->currentWallet->transactionHistoryModel();
        if(model != nullptr) {
            model->preferredFiatSymbol = symbol;
        }
    }
}

void AppContext::onWalletOpened(Wallet *wallet) {
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        auto errMsg = wallet->errorString();
        if(errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc")) {
            qCritical() << errMsg;
            this->walletManager->clearWalletCache(this->walletPath);
            errMsg = QString("%1\n\nAttempted to clean wallet cache. Please restart Feather.").arg(errMsg);
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else if(errMsg.contains("wallet cannot be opened as")) {
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else if(errMsg.contains("is opened by another wallet program")) {
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else {
            this->closeWallet(false);
            emit walletOpenPasswordNeeded(!this->walletPassword.isEmpty(), wallet->path());
        }
        return;
    }

    this->refreshed = false;
    this->currentWallet = wallet;
    this->walletPath = this->currentWallet->path() + ".keys";
    this->walletViewOnly = this->currentWallet->viewOnly();
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
    this->setWindowTitle();
}

void AppContext::setWindowTitle(bool mining) {
    QFileInfo fileInfo(this->walletPath);
    auto title = QString("Feather - [%1]").arg(fileInfo.fileName());
    if(this->walletViewOnly)
        title += " [view-only]";
    if(mining)
        title += " [mining]";

    emit setTitle(title);
}

void AppContext::onWSMessage(const QJsonObject &msg) {
    QString cmd = msg.value("cmd").toString();

    if(cmd == "blockheights") {
        auto heights = msg.value("data").toObject();
        auto mainnet = heights.value("mainnet").toInt();
        auto stagenet = heights.value("stagenet").toInt();
        auto changed = false;

        if(!this->heights.contains("mainnet")) {
            this->heights["mainnet"] = mainnet;
            changed = true;
        }
        else {
            if (mainnet > this->heights["mainnet"]) {
                this->heights["mainnet"] = mainnet;
                changed = true;
            }
        }
        if(!this->heights.contains("stagenet")) {
            this->heights["stagenet"] = stagenet;
            changed = true;
        }
        else {
            if (stagenet > this->heights["stagenet"]) {
                this->heights["stagenet"] = stagenet;
                changed = true;
            }
        }

        if(changed)
            emit blockHeightWSUpdated(this->heights);
    }

    else if(cmd == "nodes") {
        this->onWSNodes(msg.value("data").toArray());
    }
#if defined(HAS_XMRIG)
    else if(cmd == "xmrig") {
        this->XMRigDownloads(msg.value("data").toObject());
    }
#endif
    else if(cmd == "crypto_rates") {
        QJsonArray crypto_rates = msg.value("data").toArray();
        AppContext::prices->cryptoPricesReceived(crypto_rates);
    }

    else if(cmd == "fiat_rates") {
        QJsonObject fiat_rates = msg.value("data").toObject();
        AppContext::prices->fiatPricesReceived(fiat_rates);
    }
#if defined(HAS_XMRTO)
    else if(cmd == "xmrto_rates") {
        auto xmr_rates = msg.value("data").toObject();
        this->XMRTo->onRatesReceived(xmr_rates);
    }
#endif
    else if(cmd == "reddit") {
        QJsonArray reddit_data = msg.value("data").toArray();
        this->onWSReddit(reddit_data);
    }

    else if(cmd == "ccs") {
        auto ccs_data = msg.value("data").toArray();
        this->onWSCCS(ccs_data);
    }

    else if(cmd == "txFiatHistory") {
        auto txFiatHistory_data = msg.value("data").toObject();
        AppContext::txFiatHistory->onWSData(txFiatHistory_data);
    }
}

void AppContext::onWSNodes(const QJsonArray &nodes) {
    QList<QSharedPointer<FeatherNode>> l;
    for (auto &&entry: nodes) {
        auto obj = entry.toObject();
        auto nettype = obj.value("nettype");
        auto type = obj.value("type");

        // filter remote node network types
        if(nettype == "mainnet" && this->networkType != NetworkType::MAINNET)
            continue;
        if(nettype == "stagenet" && this->networkType != NetworkType::STAGENET)
            continue;
        if(nettype == "testnet" && this->networkType != NetworkType::TESTNET)
            continue;

        if(type == "clearnet" && (this->isTails || this->isWhonix || this->isTorSocks))
            continue;
        if(type == "tor" && (!(this->isTails || this->isWhonix || this->isTorSocks)))
            continue;

        auto node = new FeatherNode(
                obj.value("address").toString(),
                 obj.value("height").toInt(),
                 obj.value("target_height").toInt(),
                obj.value("online").toBool());
        QSharedPointer<FeatherNode> r = QSharedPointer<FeatherNode>(node);
        l.append(r);
    }
    this->nodes->onWSNodesReceived(l);
}

void AppContext::onWSReddit(const QJsonArray& reddit_data) {
    QList<QSharedPointer<RedditPost>> l;

    for (auto &&entry: reddit_data) {
        auto obj = entry.toObject();
        auto redditPost = new RedditPost(
                obj.value("title").toString(),
                obj.value("author").toString(),
                obj.value("permalink").toString(),
                obj.value("comments").toInt());
        QSharedPointer<RedditPost> r = QSharedPointer<RedditPost>(redditPost);
        l.append(r);
    }

    emit redditUpdated(l);
}

void AppContext::onWSCCS(const QJsonArray &ccs_data) {
    QList<QSharedPointer<CCSEntry>> l;


    QStringList fonts = {"state", "address", "author", "date",
                         "title", "target_amount", "raised_amount",
                         "percentage_funded", "contributions"};

    for (auto &&entry: ccs_data) {
        auto obj = entry.toObject();
        auto c = QSharedPointer<CCSEntry>(new CCSEntry());

        if (obj.value("state").toString() != "FUNDING-REQUIRED")
            continue;

        c->state = obj.value("state").toString();
        c->address = obj.value("address").toString();
        c->author = obj.value("author").toString();
        c->date = obj.value("date").toString();
        c->title = obj.value("title").toString();
        c->url = obj.value("url").toString();
        c->target_amount = obj.value("target_amount").toDouble();
        c->raised_amount = obj.value("raised_amount").toDouble();
        c->percentage_funded = obj.value("percentage_funded").toDouble();
        c->contributions = obj.value("contributions").toInt();
        l.append(c);
    }

    emit ccsUpdated(l);
}

void AppContext::createConfigDirectory(const QString &dir) {
    QString config_dir_tor = QString("%1%2").arg(dir).arg("tor");
    QString config_dir_tordata = QString("%1%2").arg(dir).arg("tor/data");

    QStringList createDirs({dir, config_dir_tor, config_dir_tordata});
    for(const auto &d: createDirs) {
        if(!Utils::dirExists(d)) {
            qDebug() << QString("Creating directory: %1").arg(d);
            if (!QDir().mkpath(d))
                throw std::runtime_error("Could not create directory " + d.toStdString());
        }
    }
}

void AppContext::createWallet(FeatherSeed seed, const QString &path, const QString &password) {
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
        wallet = this->walletManager->createDeterministicWalletFromSpendKey(path, password, seed.language, this->networkType, seed.spendKey, seed.restoreHeight, this->kdfRounds);
        wallet->setCacheAttribute("feather.seed", seed.mnemonic.join(" "));
    }
    if (seed.seedType == SeedType::MONERO) {
        wallet = this->walletManager->recoveryWallet(path, password, seed.mnemonic.join(" "), "", this->networkType, seed.restoreHeight, this->kdfRounds);
    }

    this->currentWallet = wallet;
    if(this->currentWallet == nullptr) {
        emit walletCreatedError("Failed to write wallet");
        return;
    }

    this->createWalletFinish(password);
}

void AppContext::createWalletViewOnly(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!this->walletManager->addressValid(address, this->networkType)) {
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

    this->currentWallet = this->walletManager->createWalletFromKeys(path, this->seedLanguage, this->networkType, address, viewkey, spendkey, restoreHeight);
    this->createWalletFinish(password);
}

void AppContext::createWalletFinish(const QString &password) {
    this->currentWallet->setPassword(password);
    this->currentWallet->store();
    this->walletPassword = password;
    emit walletCreated(this->currentWallet);
}

void AppContext::initRestoreHeights() {
    restoreHeights[NetworkType::TESTNET] = new RestoreHeightLookup(NetworkType::TESTNET);
    restoreHeights[NetworkType::STAGENET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_monero_stagenet.txt", NetworkType::STAGENET);
    restoreHeights[NetworkType::MAINNET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_monero_mainnet.txt", NetworkType::MAINNET);
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
    if(this->currentWallet == nullptr) return;
    if(this->networkType != NetworkType::Type::MAINNET) return;
    if(this->currentWallet->viewOnly()) return;

    auto donationCounter = config()->get(Config::donateBeg).toInt();
    if(donationCounter == -1)
        return;  // previously donated

    donationCounter += 1;
    if (donationCounter % m_donationBoundary == 0)
        emit donationNag();
    config()->set(Config::donateBeg, donationCounter);
}

AppContext::~AppContext() {}

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

void AppContext::onWalletRefreshed(bool success) {
    if (!this->refreshed) {
        refreshModels();
        this->refreshed = true;
        emit walletRefreshed();
        // store wallet immediately upon finishing synchronization
        this->currentWallet->store();
    }

    qDebug() << "Wallet refresh status: " << success;

    this->currentWallet->refreshHeightAsync();
}

void AppContext::onWalletNewBlock(quint64 blockheight, quint64 targetHeight) {
    this->syncStatusUpdated(blockheight, targetHeight);

    if (this->currentWallet->synchronized()) {
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
    for (auto &addr : address) {
        if (addr == this->donationAddress) {
            this->donationSending = true;
        }
    }

    // Let UI know that the transaction was constructed
    emit endTransaction();

    // tx created, but not sent yet. ask user to verify first.
    emit createTransactionSuccess(tx, address);
}

void AppContext::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid){
    this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->coins()->refresh(this->currentWallet->currentSubaddressAccount());

    // Store wallet immediately so we don't risk losing tx key if wallet crashes
    this->currentWallet->store();

    this->updateBalance();

    emit transactionCommitted(status, tx, txid);

    // this tx was a donation to Feather, stop our nagging
    if(this->donationSending) {
        this->donationSending = false;
        config()->set(Config::donateBeg, -1);
    }
}

void AppContext::storeWallet() {
    // Do not store a synchronizing wallet: store() is NOT thread safe and may crash the wallet
    if (this->currentWallet == nullptr || !this->currentWallet->synchronized())
        return;

    qDebug() << "Storing wallet";
    this->currentWallet->store();
}

void AppContext::updateBalance() {
    if (!this->currentWallet)
        return;

    quint64 balance_u = this->currentWallet->balance();
    AppContext::balance = balance_u / globals::cdiv;
    double spendable = this->currentWallet->unlockedBalance();

    emit balanceUpdated(balance_u, spendable);
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
