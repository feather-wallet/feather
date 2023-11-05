// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Wallet.h"

#include <chrono>
#include <thread>

#include "AddressBook.h"
#include "Coins.h"
#include "Subaddress.h"
#include "SubaddressAccount.h"
#include "TransactionHistory.h"
#include "WalletManager.h"

#include "config.h"
#include "constants.h"

#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "model/AddressBookModel.h"
#include "model/SubaddressModel.h"
#include "model/SubaddressAccountModel.h"
#include "model/CoinsModel.h"

#include "utils/ScopeGuard.h"

namespace {
    constexpr char ATTRIBUTE_SUBADDRESS_ACCOUNT[] = "feather.subaddress_account";
}

Wallet::Wallet(Monero::Wallet *wallet, QObject *parent)
        : QObject(parent)
        , m_walletImpl(wallet)
        , m_history(new TransactionHistory(m_walletImpl->history(), this))
        , m_historyModel(nullptr)
        , m_addressBook(new AddressBook(m_walletImpl->addressBook(), this))
        , m_addressBookModel(nullptr)
        , m_daemonBlockChainHeight(0)
        , m_daemonBlockChainTargetHeight(0)
        , m_connectionStatus(Wallet::ConnectionStatus_Disconnected)
        , m_currentSubaddressAccount(0)
        , m_subaddress(new Subaddress(m_walletImpl->subaddress(), this))
        , m_subaddressAccount(new SubaddressAccount(m_walletImpl->subaddressAccount(), this))
        , m_refreshNow(false)
        , m_refreshEnabled(false)
        , m_scheduler(this)
        , m_useSSL(true)
        , m_coins(new Coins(m_walletImpl->coins(), this))
        , m_storeTimer(new QTimer(this))
{
    m_walletListener = new WalletListenerImpl(this);
    m_walletImpl->setListener(m_walletListener);
    m_currentSubaddressAccount = getCacheAttribute(ATTRIBUTE_SUBADDRESS_ACCOUNT).toUInt();

    m_addressBookModel = new AddressBookModel(this, m_addressBook);
    m_subaddressModel = new SubaddressModel(this, m_subaddress);
    m_subaddressAccountModel = new SubaddressAccountModel(this, m_subaddressAccount);
    m_coinsModel = new CoinsModel(this, m_coins);

    if (this->status() == Status_Ok) {
        startRefreshThread();

        // Store the wallet every 2 minutes
        m_storeTimer->start(2 * 60 * 1000);
        connect(m_storeTimer, &QTimer::timeout, [this](){
            this->storeSafer();
        });

        this->updateBalance();
    }

    connect(this->history(), &TransactionHistory::txNoteChanged, [this]{
        this->history()->refresh(this->currentSubaddressAccount());
    });

    connect(this, &Wallet::refreshed, this, &Wallet::onRefreshed);
    connect(this, &Wallet::newBlock, this, &Wallet::onNewBlock);
    connect(this, &Wallet::updated, this, &Wallet::onUpdated);
    connect(this, &Wallet::heightsRefreshed, this, &Wallet::onHeightsRefreshed);
    connect(this, &Wallet::transactionCommitted, this, &Wallet::onTransactionCommitted);
}

// #################### Status ####################

Wallet::Status Wallet::status() const {
    return static_cast<Status>(m_walletImpl->status());
}

Wallet::ConnectionStatus Wallet::connectionStatus() const {
    return m_connectionStatus;
}

void Wallet::setConnectionStatus(ConnectionStatus value) {
    if (m_connectionStatus == value) {
        return;
    }

    m_connectionStatus = value;
    emit connectionStatusChanged(m_connectionStatus);
}

bool Wallet::isSynchronized() const {
    return connectionStatus() == ConnectionStatus_Synchronized;
}

bool Wallet::isConnected() const {
    auto status = connectionStatus();
    return status == ConnectionStatus_Synchronizing || status == ConnectionStatus_Synchronized;
}

QString Wallet::errorString() const {
    return QString::fromStdString(m_walletImpl->errorString());
}

NetworkType::Type Wallet::nettype() const {
    return static_cast<NetworkType::Type>(m_walletImpl->nettype());
}

bool Wallet::viewOnly() const {
    return m_walletImpl->watchOnly();
}

bool Wallet::isDeterministic() const {
    return m_walletImpl->isDeterministic();
}

QString Wallet::walletName() const {
    return QFileInfo(this->cachePath()).fileName();
}

// #################### Balance ####################

quint64 Wallet::balance() const {
    return balance(m_currentSubaddressAccount);
}

quint64 Wallet::balance(quint32 accountIndex) const {
    return m_walletImpl->balance(accountIndex);
}

quint64 Wallet::balanceAll() const {
    return m_walletImpl->balanceAll();
}

quint64 Wallet::unlockedBalance() const {
    return unlockedBalance(m_currentSubaddressAccount);
}

quint64 Wallet::unlockedBalance(quint32 accountIndex) const {
    return m_walletImpl->unlockedBalance(accountIndex);
}

quint64 Wallet::unlockedBalanceAll() const {
    return m_walletImpl->unlockedBalanceAll();
}

quint64 Wallet::viewOnlyBalance(quint32 accountIndex) const {
    std::vector<std::string> kis;
    for (const auto & ki : m_selectedInputs) {
        kis.push_back(ki);
    }
    return m_walletImpl->viewOnlyBalance(accountIndex, kis);
}

void Wallet::updateBalance() {
    quint64 balance = this->balance();
    quint64 spendable = this->unlockedBalance();

    emit balanceUpdated(balance, spendable);
}

// #################### Subaddresses and Accounts ####################

QString Wallet::address(quint32 accountIndex, quint32 addressIndex) const {
    return QString::fromStdString(m_walletImpl->address(accountIndex, addressIndex));
}

SubaddressIndex Wallet::subaddressIndex(const QString &address) const {
    std::pair<uint32_t, uint32_t> i;
    if (!m_walletImpl->subaddressIndex(address.toStdString(), i)) {
        return SubaddressIndex(-1, -1);
    }
    return SubaddressIndex(i.first, i.second);
}

quint32 Wallet::currentSubaddressAccount() const {
    return m_currentSubaddressAccount;
}

void Wallet::switchSubaddressAccount(quint32 accountIndex) {
    if (accountIndex < numSubaddressAccounts())
    {
        m_currentSubaddressAccount = accountIndex;
        if (!setCacheAttribute(ATTRIBUTE_SUBADDRESS_ACCOUNT, QString::number(m_currentSubaddressAccount)))
        {
            qWarning() << "failed to set " << ATTRIBUTE_SUBADDRESS_ACCOUNT << " cache attribute";
        }
        m_subaddress->refresh(m_currentSubaddressAccount);
        m_history->refresh(m_currentSubaddressAccount);
        m_coins->refresh(m_currentSubaddressAccount);
        this->subaddressModel()->setCurrentSubaddressAccount(m_currentSubaddressAccount);
        this->coinsModel()->setCurrentSubaddressAccount(m_currentSubaddressAccount);
        this->updateBalance();
        emit currentSubaddressAccountChanged();
    }
}
void Wallet::addSubaddressAccount(const QString& label) {
    m_walletImpl->addSubaddressAccount(label.toStdString());
    switchSubaddressAccount(numSubaddressAccounts() - 1);
}

quint32 Wallet::numSubaddressAccounts() const {
    return m_walletImpl->numSubaddressAccounts();
}

quint32 Wallet::numSubaddresses(quint32 accountIndex) const {
    return m_walletImpl->numSubaddresses(accountIndex);
}

void Wallet::addSubaddress(const QString& label) {
    m_walletImpl->addSubaddress(currentSubaddressAccount(), label.toStdString());
}

QString Wallet::getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const {
    return QString::fromStdString(m_walletImpl->getSubaddressLabel(accountIndex, addressIndex));
}

void Wallet::setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label) {
    m_walletImpl->setSubaddressLabel(accountIndex, addressIndex, label.toStdString());
}

void Wallet::deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId) {
    m_scheduler.run([this, accountIndex, addressIndex, paymentId] {
        m_walletImpl->deviceShowAddress(accountIndex, addressIndex, paymentId.toStdString());
        emit deviceShowAddressShowed();
    });
}

QString Wallet::getSubaddressLookahead() const {
    auto lookahead = m_walletImpl->getSubaddressLookahead();

    return QString("%1:%2").arg(QString::number(lookahead.first), QString::number(lookahead.second));
}

// #################### Seed ####################

QString Wallet::getSeed(const QString &seedOffset) const {
    return QString::fromStdString(m_walletImpl->seed(seedOffset.toStdString()));
}

qsizetype Wallet::seedLength() const {
    auto seedLength = this->getCacheAttribute("feather.seed").split(" ", Qt::SkipEmptyParts).length();
    return seedLength ? seedLength : 25;
}

QString Wallet::getSeedLanguage() const
{
    return QString::fromStdString(m_walletImpl->getSeedLanguage());
}

void Wallet::setSeedLanguage(const QString &lang)
{
    m_walletImpl->setSeedLanguage(lang.toStdString());
}

// #################### Node connection ####################

void Wallet::setOffline(bool offline) const {
    return m_walletImpl->setOffline(offline);
}

void Wallet::setTrustedDaemon(bool arg) {
    m_walletImpl->setTrustedDaemon(arg);
}

void Wallet::setUseSSL(bool ssl) {
    m_useSSL = ssl;
}

void Wallet::setDaemonLogin(const QString &daemonUsername, const QString &daemonPassword) {
    m_daemonUsername = daemonUsername;
    m_daemonPassword = daemonPassword;
}

void Wallet::initAsync(const QString &daemonAddress, bool trustedDaemon, quint64 upperTransactionLimit, const QString &proxyAddress)
{
    qDebug() << "initAsync: " + daemonAddress;
    const auto future = m_scheduler.run([this, daemonAddress, trustedDaemon, upperTransactionLimit, proxyAddress] {
        // Beware! This code does not run in the GUI thread.

        bool success;
        {
            QMutexLocker locker(&m_proxyMutex);
            success = m_walletImpl->init(daemonAddress.toStdString(), upperTransactionLimit, m_daemonUsername.toStdString(), m_daemonPassword.toStdString(), m_useSSL, false, proxyAddress.toStdString());
        }

        setTrustedDaemon(trustedDaemon);

        if (success) {
            qDebug() << "init async finished - starting refresh";
            startRefresh();
        }
    });
    if (future.first)
    {
        setConnectionStatus(Wallet::ConnectionStatus_Connecting);
    }
}

// #################### Synchronization (Refresh) ####################

void Wallet::startRefresh() {
    m_refreshEnabled = true;
    m_refreshNow = true;
}

void Wallet::pauseRefresh() {
    m_refreshEnabled = false;
}

void Wallet::startRefreshThread()
{
    const auto future = m_scheduler.run([this] {
        // Beware! This code does not run in the GUI thread.

        constexpr const std::chrono::seconds refreshInterval{10};
        constexpr const std::chrono::milliseconds intervalResolution{100};

        auto last = std::chrono::steady_clock::now();
        while (!m_scheduler.stopping())
        {
            if (m_refreshEnabled && (!isHwBacked() || isDeviceConnected()))
            {
                const auto now = std::chrono::steady_clock::now();
                const auto elapsed = now - last;
                if (elapsed >= refreshInterval || m_refreshNow)
                {
                    m_refreshNow = false;

                    // get daemonHeight and targetHeight
                    // daemonHeight and targetHeight will be 0 if call to get_info fails
                    quint64 daemonHeight = m_walletImpl->daemonBlockChainHeight();
                    bool success = daemonHeight > 0;

                    quint64 targetHeight = 0;
                    if (success) {
                        targetHeight = m_walletImpl->daemonBlockChainTargetHeight();
                    }
                    bool haveHeights = (daemonHeight > 0 && targetHeight > 0);

                    emit heightsRefreshed(haveHeights, daemonHeight, targetHeight);

                    // Don't call refresh function if we don't have the daemon and target height
                    // We do this to prevent to UI from getting confused about the amount of blocks that are still remaining
                    if (haveHeights) {
                        QMutexLocker locker(&m_asyncMutex);

                        if (m_newWallet) {
                            // Set blockheight to daemonHeight for newly created wallets to speed up initial sync
                            m_walletImpl->setRefreshFromBlockHeight(daemonHeight);
                            m_newWallet = false;
                        }

                        m_walletImpl->refresh();
                    }
                    last = std::chrono::steady_clock::now();
                }
            }

            std::this_thread::sleep_for(intervalResolution);
        }
    });
    if (!future.first)
    {
        throw std::runtime_error("failed to start auto refresh thread");
    }
}

void Wallet::onHeightsRefreshed(bool success, quint64 daemonHeight, quint64 targetHeight) {
    m_daemonBlockChainHeight = daemonHeight;
    m_daemonBlockChainTargetHeight = targetHeight;

    if (success) {
        quint64 walletHeight = blockChainHeight();

        if (daemonHeight < targetHeight) {
            emit blockchainSync(daemonHeight, targetHeight);
        }
        else {
            this->syncStatusUpdated(walletHeight, daemonHeight);
        }

        if (walletHeight < (daemonHeight - 1)) {
            setConnectionStatus(ConnectionStatus_Synchronizing);
        } else {
            setConnectionStatus(ConnectionStatus_Synchronized);
        }
    } else {
        setConnectionStatus(ConnectionStatus_Disconnected);
    }
}

quint64 Wallet::blockChainHeight() const {
    // Can not block UI
    return m_walletImpl->blockChainHeight();
}

quint64 Wallet::daemonBlockChainHeight() const {
    return m_daemonBlockChainHeight;
}

quint64 Wallet::daemonBlockChainTargetHeight() const {
    return m_daemonBlockChainTargetHeight;
}

void Wallet::syncStatusUpdated(quint64 height, quint64 target) {
    if (height < (target - 1)) {
        emit refreshSync(height, target);
    }
    else {
        this->updateBalance();
        emit synchronized();
    }
}

void Wallet::onNewBlock(uint64_t walletHeight) {
    // Called whenever a new block gets scanned by the wallet
    quint64 daemonHeight = m_daemonBlockChainTargetHeight;

    if (walletHeight < (daemonHeight - 1)) {
        setConnectionStatus(ConnectionStatus_Synchronizing);
    } else {
        setConnectionStatus(ConnectionStatus_Synchronized);
    }

    this->syncStatusUpdated(walletHeight, daemonHeight);

    if (this->isSynchronized()) {
        this->refreshModels();
    }
}

void Wallet::onUpdated() {
    this->updateBalance();
    if (this->isSynchronized()) {
        this->refreshModels();
    }
}

void Wallet::onRefreshed(bool success, const QString &message) {
    if (!success) {
        setConnectionStatus(ConnectionStatus_Disconnected);
        // Something went wrong during refresh, in some cases we need to notify the user
        qCritical() << "Exception during refresh: " << message; // Can't use ->errorString() here, other SLOT might snipe it first
        return;
    }

    if (!this->refreshedOnce) {
        this->refreshedOnce = true;
        emit walletRefreshed();
        // store wallet immediately upon finishing synchronization
        this->storeSafer();
    }
}

void Wallet::refreshModels() {
    m_history->refresh(this->currentSubaddressAccount());
    m_coins->refresh(this->currentSubaddressAccount());
    bool r = this->subaddress()->refresh(this->currentSubaddressAccount());

    if (!r) {
        // This should only happen if wallet keys got corrupted or were tampered with
        // The list of subaddresses is wiped to prevent loss of funds
        // Notify MainWindow to display an error message
        emit keysCorrupted();
    }
}

// #################### Hardware wallet ####################

bool Wallet::isHwBacked() const {
    return m_walletImpl->getDeviceType() != Monero::Wallet::Device_Software;
}

bool Wallet::isLedger() const {
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Ledger;
}

bool Wallet::isTrezor() const {
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Trezor;
}

bool Wallet::isDeviceConnected() const {
    return m_walletImpl->isDeviceConnected();
}

bool Wallet::reconnectDevice() {
    return m_walletImpl->reconnectDevice();
}

void Wallet::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) {
    if (m_walletListener != nullptr) {
        m_walletListener->onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }
}

void Wallet::onWalletPassphraseNeeded(bool on_device) {
    emit this->walletPassphraseNeeded(on_device);
}

// #################### Import / Export ####################

bool Wallet::hasUnknownKeyImages() const {
    return m_walletImpl->hasUnknownKeyImages();
}

bool Wallet::exportKeyImages(const QString& path, bool all) {
    return m_walletImpl->exportKeyImages(path.toStdString(), all);
}

bool Wallet::exportKeyImagesToStr(std::string &keyImages, bool all) {
    return m_walletImpl->exportKeyImagesToStr(keyImages, all);
}

bool Wallet::importKeyImages(const QString& path) {
    return m_walletImpl->importKeyImages(path.toStdString());
}

bool Wallet::importKeyImagesFromStr(const std::string &keyImages) {
    return m_walletImpl->importKeyImagesFromStr(keyImages);
}

bool Wallet::exportOutputs(const QString& path, bool all) {
    return m_walletImpl->exportOutputs(path.toStdString(), all);
}

bool Wallet::exportOutputsToStr(std::string& outputs, bool all) {
    return m_walletImpl->exportOutputsToStr(outputs, all);
}

bool Wallet::importOutputs(const QString& path) {
    return m_walletImpl->importOutputs(path.toStdString());
}

bool Wallet::importOutputsFromStr(const std::string &outputs) {
    return m_walletImpl->importOutputsFromStr(outputs);
}

bool Wallet::importTransaction(const QString& txid) {
    std::vector<std::string> txids = {txid.toStdString()};
    return m_walletImpl->scanTransactions(txids);
}

// #################### Wallet cache ####################

void Wallet::store() {
    m_walletImpl->store();
}

void Wallet::storeSafer() {
    // Do not store a synchronizing wallet: store() is NOT thread safe and may crash the wallet
    if (!this->isSynchronized()) {
        return;
    }

    qDebug() << "Storing wallet";
    this->store();
}

QString Wallet::cachePath() const {
    return QDir::toNativeSeparators(QString::fromStdString(m_walletImpl->filename()));
}

QString Wallet::keysPath() const {
    return QDir::toNativeSeparators(QString::fromStdString(m_walletImpl->keysFilename()));;
}

bool Wallet::setPassword(const QString &oldPassword, const QString &newPassword) {
    return m_walletImpl->setPassword(oldPassword.toStdString(), newPassword.toStdString());
}

bool Wallet::verifyPassword(const QString &password) {
    return m_walletImpl->verifyPassword(password.toStdString());
}

bool Wallet::cacheAttributeExists(const QString &key) {
    return m_walletImpl->cacheAttributeExists(key.toStdString());
}

bool Wallet::setCacheAttribute(const QString &key, const QString &val) {
    return m_walletImpl->setCacheAttribute(key.toStdString(), val.toStdString());
}

QString Wallet::getCacheAttribute(const QString &key) const {
    return QString::fromStdString(m_walletImpl->getCacheAttribute(key.toStdString()));
}

void Wallet::addCacheTransaction(const QString &txid, const QString &txHex) {
    this->setCacheAttribute(QString("tx:%1").arg(txid), txHex);
}

QString Wallet::getCacheTransaction(const QString &txid) const {
    return this->getCacheAttribute(QString("tx:%1").arg(txid));
}

bool Wallet::setUserNote(const QString &txid, const QString &note) {
    return m_walletImpl->setUserNote(txid.toStdString(), note.toStdString());
}

QString Wallet::getUserNote(const QString &txid) const {
    return QString::fromStdString(m_walletImpl->getUserNote(txid.toStdString()));
}

QString Wallet::printBlockchain() {
    return QString::fromStdString(m_walletImpl->printBlockchain());
}

QString Wallet::printTransfers() {
    return QString::fromStdString(m_walletImpl->printTransfers());
}

QString Wallet::printPayments() {
    return QString::fromStdString(m_walletImpl->printPayments());
}

QString Wallet::printUnconfirmedPayments() {
    return QString::fromStdString(m_walletImpl->printUnconfirmedPayments());
}

QString Wallet::printConfirmedTransferDetails() {
    return QString::fromStdString(m_walletImpl->printConfirmedTransferDetails());
}

QString Wallet::printUnconfirmedTransferDetails() {
    return QString::fromStdString(m_walletImpl->printUnconfirmedTransferDetails());
}

QString Wallet::printPubKeys() {
    return QString::fromStdString(m_walletImpl->printPubKeys());
}

QString Wallet::printTxNotes() {
    return QString::fromStdString(m_walletImpl->printTxNotes());
}

QString Wallet::printSubaddresses() {
    return QString::fromStdString(m_walletImpl->printSubaddresses());
}

QString Wallet::printSubaddressLabels() {
    return QString::fromStdString(m_walletImpl->printSubaddressLabels());
}

QString Wallet::printAdditionalTxKeys() {
    return QString::fromStdString(m_walletImpl->printAdditionalTxKeys());
}

QString Wallet::printAttributes() {
    return QString::fromStdString(m_walletImpl->printAttributes());
}

QString Wallet::printKeyImages() {
    return QString::fromStdString(m_walletImpl->printKeyImages());
}

QString Wallet::printAccountTags() {
    return QString::fromStdString(m_walletImpl->printAccountTags());
}

QString Wallet::printTxKeys() {
    return QString::fromStdString(m_walletImpl->printTxKeys());
}

QString Wallet::printAddressBook() {
    return QString::fromStdString(m_walletImpl->printAddressBook());
}

QString Wallet::printScannedPoolTxs() {
    return QString::fromStdString(m_walletImpl->printScannedPoolTxs());
}

// #################### Transactions ####################

// Phase 0: Pre-construction setup

void Wallet::setSelectedInputs(const QStringList &selectedInputs) {
    m_selectedInputs.clear();
    for (const auto &input : selectedInputs) {
        m_selectedInputs.insert(input.toStdString());
    }
    emit selectedInputsChanged(selectedInputs);
}

// Phase 1: Transaction creation
// Pick one:
// - createTransaction
// - createTransactionMultiDest
// - sweepOutputs

void Wallet::createTransaction(const QString &address, quint64 amount, const QString &description, bool all) {
    this->tmpTxDescription = description;

    qInfo() << "Creating transaction";
    m_scheduler.run([this, all, address, amount] {
        std::set<uint32_t> subaddr_indices;

        Monero::PendingTransaction *ptImpl = m_walletImpl->createTransaction(address.toStdString(), "", all ? Monero::optional<uint64_t>() : Monero::optional<uint64_t>(amount), constants::mixin,
                                                                             static_cast<Monero::PendingTransaction::Priority>(this->tx_priority),
                                                                             currentSubaddressAccount(), subaddr_indices, m_selectedInputs);

        QVector<QString> addresses{address};
        this->onTransactionCreated(ptImpl, addresses);
    });

    emit initiateTransaction();
}

void Wallet::createTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description) {
    this->tmpTxDescription = description;

    qInfo() << "Creating transaction";
    m_scheduler.run([this, addresses, amounts] {
        std::vector<std::string> dests;
        for (auto &addr : addresses) {
            dests.push_back(addr.toStdString());
        }

        std::vector<uint64_t> amount;
        for (auto &a : amounts) {
            amount.push_back(a);
        }

        std::set<uint32_t> subaddr_indices;
        Monero::PendingTransaction *ptImpl = m_walletImpl->createTransactionMultDest(dests, "", amount, constants::mixin,
                                                                                     static_cast<Monero::PendingTransaction::Priority>(this->tx_priority),
                                                                                     currentSubaddressAccount(), subaddr_indices, m_selectedInputs);

        this->onTransactionCreated(ptImpl, addresses);
    });

    emit initiateTransaction();
}

void Wallet::sweepOutputs(const QVector<QString> &keyImages, QString address, bool churn, int outputs) {
    if (churn) {
        address = this->address(0, 0);
    }

    qInfo() << "Creating transaction";
    m_scheduler.run([this, keyImages, address, outputs] {
        std::vector<std::string> kis;
        for (const auto &key_image : keyImages) {
            kis.push_back(key_image.toStdString());
        }
        Monero::PendingTransaction *ptImpl = m_walletImpl->createTransactionSelected(kis, address.toStdString(), outputs, static_cast<Monero::PendingTransaction::Priority>(this->tx_priority));

        QVector<QString> addresses {address};
        this->onTransactionCreated(ptImpl, addresses);
    });

    emit initiateTransaction();
}

// Phase 2: Transaction construction completed

void Wallet::onTransactionCreated(Monero::PendingTransaction *mtx, const QVector<QString> &address) {
    qDebug() << Q_FUNC_INFO;

    PendingTransaction *tx = new PendingTransaction(mtx, this);

    for (auto &addr : address) {
        if (addr == constants::donationAddress) {
            this->donationSending = true;
        }
    }

    // tx created, but not sent yet. ask user to verify first.
    emit transactionCreated(tx, address);
}

// Phase 3: Commit or dispose

void Wallet::commitTransaction(PendingTransaction *tx, const QString &description) {
    // Clear list of selected transfers
    this->setSelectedInputs({});

    QMap<QString, QString> txHexMap;
    for (int i = 0; i < tx->txCount(); i++) {
        txHexMap[tx->txid()[i]] = tx->signedTxToHex(i);
    }

    m_scheduler.run([this, tx, description, txHexMap] {
        auto txIdList = tx->txid();  // retrieve before commit
        bool success = tx->commit();

        if (success && !description.isEmpty()) {
            for (const auto &txid : txIdList) {
                this->setUserNote(txid, description);
            }
        }

        emit transactionCommitted(success, tx, txIdList, txHexMap);
    });
}

void Wallet::onTransactionCommitted(bool success, PendingTransaction *tx, const QStringList &txid, const QMap<QString, QString> &txHexMap) {
    // Store wallet immediately, so we don't risk losing tx key if wallet crashes
    this->storeSafer();

    this->history()->refresh(this->currentSubaddressAccount());
    this->coins()->refresh(this->currentSubaddressAccount());
    this->updateBalance();

    if (!success) {
        return;
    }

    // Nodes - even well-connected, properly configured ones - consistently fail to relay transactions
    // To mitigate transactions failing we just send the transaction to every node we know about over Tor
    if (conf()->get(Config::multiBroadcast).toBool()) {
        // Let MainWindow handle this
        emit multiBroadcast(txHexMap);
    }

    // this tx was a donation to Feather, stop our nagging
    if (this->donationSending) {
        this->donationSending = false;
        emit donationSent();
    }
}

void Wallet::disposeTransaction(PendingTransaction *t) {
    m_walletImpl->disposeTransaction(t->m_pimpl);
    delete t;
}

void Wallet::disposeTransaction(UnsignedTransaction *t) {
    delete t;
}

// #################### Transaction import ####################

bool Wallet::haveTransaction(const QString &txid)
{
    return m_walletImpl->haveTransaction(txid.toStdString());
}

UnsignedTransaction * Wallet::loadTxFile(const QString &fileName)
{
    qDebug() << "Trying to sign " << fileName;
    Monero::UnsignedTransaction *ptImpl = m_walletImpl->loadUnsignedTx(fileName.toStdString());
    UnsignedTransaction *result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

UnsignedTransaction * Wallet::loadUnsignedTransactionFromStr(const std::string &data) {
    Monero::UnsignedTransaction *ptImpl = m_walletImpl->loadUnsignedTxFromStr(data);
    UnsignedTransaction *result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

UnsignedTransaction * Wallet::loadTxFromBase64Str(const QString &unsigned_tx)
{
    Monero::UnsignedTransaction *ptImpl = m_walletImpl->loadUnsignedTxFromBase64Str(unsigned_tx.toStdString());
    UnsignedTransaction *result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

PendingTransaction * Wallet::loadSignedTxFile(const QString &fileName)
{
    qDebug() << "Tying to load " << fileName;
    Monero::PendingTransaction *ptImpl = m_walletImpl->loadSignedTx(fileName.toStdString());
    PendingTransaction *result = new PendingTransaction(ptImpl, this);
    return result;
}

PendingTransaction * Wallet::loadSignedTxFromStr(const std::string &data)
{
    Monero::PendingTransaction *ptImpl = m_walletImpl->loadSignedTxFromStr(data);
    PendingTransaction *result = new PendingTransaction(ptImpl, this);
    return result;
}

bool Wallet::submitTxFile(const QString &fileName) const
{
    qDebug() << "Trying to submit " << fileName;
    if (!m_walletImpl->submitTransaction(fileName.toStdString()))
        return false;
    // import key images
    return m_walletImpl->importKeyImages(fileName.toStdString() + "_keyImages");
}

// #################### Models ####################

TransactionHistory *Wallet::history() const {
    return m_history;
}

TransactionHistoryProxyModel *Wallet::historyModel()
{
    if (!m_historyModel) {
        Wallet *w = const_cast<Wallet*>(this);
        m_historyModel = new TransactionHistoryModel(w);
        m_historyModel->setTransactionHistory(this->history());
        m_historySortFilterModel = new TransactionHistoryProxyModel(w);
        m_historySortFilterModel->setSourceModel(m_historyModel);
        m_historySortFilterModel->setSortRole(TransactionHistoryModel::Date);
        m_historySortFilterModel->sort(0, Qt::DescendingOrder);
    }

    return m_historySortFilterModel;
}

TransactionHistoryModel* Wallet::transactionHistoryModel() const {
    return m_historyModel;
}

AddressBook* Wallet::addressBook() const {
    return m_addressBook;
}

AddressBookModel* Wallet::addressBookModel() const {
    return m_addressBookModel;
}

Subaddress* Wallet::subaddress() const {
    return m_subaddress;
}

SubaddressModel* Wallet::subaddressModel() const {
    return m_subaddressModel;
}

SubaddressAccount* Wallet::subaddressAccount() const {
    return m_subaddressAccount;
}

SubaddressAccountModel* Wallet::subaddressAccountModel() const {
    return m_subaddressAccountModel;
}

Coins* Wallet::coins() const {
    return m_coins;
}

CoinsModel* Wallet::coinsModel() const {
    return m_coinsModel;
}

// #################### Transaction proofs ####################

QString Wallet::getTxKey(const QString &txid) const {
    return QString::fromStdString(m_walletImpl->getTxKey(txid.toStdString()));
}

void Wallet::getTxKeyAsync(const QString &txid, const std::function<void (QVariantMap)> &callback) {
    m_scheduler.run([this, txid] {
        QVariantMap map;
        map["tx_key"] = getTxKey(txid);
        return map;
    }, callback);
}

TxKeyResult Wallet::checkTxKey(const QString &txid, const QString &tx_key, const QString &address) {
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;

    bool success = m_walletImpl->checkTxKey(txid.toStdString(), tx_key.toStdString(), address.toStdString(), received, in_pool, confirmations);
    QString errorString = success ? "" : this->errorString();
    bool good = received > 0;
    return {success, good, QString::fromStdString(Monero::Wallet::displayAmount(received)), in_pool, confirmations, errorString};
}

TxProof Wallet::getTxProof(const QString &txid, const QString &address, const QString &message) const {
    std::string result = m_walletImpl->getTxProof(txid.toStdString(), address.toStdString(), message.toStdString());
    return TxProof(QString::fromStdString(result), QString::fromStdString(m_walletImpl->errorString()));
}

TxProofResult Wallet::checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature) {
    bool good;
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
    bool success = m_walletImpl->checkTxProof(txid.toStdString(), address.toStdString(), message.toStdString(), signature.toStdString(), good, received, in_pool, confirmations);
    return {success, good, received, in_pool, confirmations};
}

void Wallet::checkTxProofAsync(const QString &txid, const QString &address, const QString &message, const QString &signature) {
    m_scheduler.run([this, txid, address, message, signature] {
        auto result = this->checkTxProof(txid, address, message, signature);
        emit transactionProofVerified(result);
    });
}

TxProof Wallet::getSpendProof(const QString &txid, const QString &message) const {
    std::string result = m_walletImpl->getSpendProof(txid.toStdString(), message.toStdString());
    return TxProof(QString::fromStdString(result), QString::fromStdString(m_walletImpl->errorString()));
}

QPair<bool, bool> Wallet::checkSpendProof(const QString &txid, const QString &message, const QString &signature) const {
    bool good;
    bool success = m_walletImpl->checkSpendProof(txid.toStdString(), message.toStdString(), signature.toStdString(), good);
    return {success, good};
}

void Wallet::checkSpendProofAsync(const QString &txid, const QString &message, const QString &signature) {
    m_scheduler.run([this, txid, message, signature] {
        auto result = this->checkSpendProof(txid, message, signature);
        emit spendProofVerified(result);
    });
}

// #################### Sign / Verify message ####################

QString Wallet::signMessage(const QString &message, bool filename, const QString &address) const {
    if (filename) {
        QFile file(message);
        uchar *data = nullptr;

        try {
            if (!file.open(QIODevice::ReadOnly))
                return "";
            quint64 size = file.size();
            if (size == 0) {
                file.close();
                return QString::fromStdString(m_walletImpl->signMessage(std::string()));
            }
            data = file.map(0, size);
            if (!data) {
                file.close();
                return "";
            }
            std::string signature = m_walletImpl->signMessage(std::string(reinterpret_cast<const char*>(data), size), address.toStdString());
            file.unmap(data);
            file.close();
            return QString::fromStdString(signature);
        }
        catch (const std::exception &e) {
            if (data)
                file.unmap(data);
            file.close();
            return "";
        }
    }
    else {
        return QString::fromStdString(m_walletImpl->signMessage(message.toStdString(), address.toStdString()));
    }
}

bool Wallet::verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename) const {
    if (filename) {
        QFile file(message);
        uchar *data = nullptr;

        try {
            if (!file.open(QIODevice::ReadOnly))
                return false;
            quint64 size = file.size();
            if (size == 0) {
                file.close();
                return m_walletImpl->verifySignedMessage(std::string(), address.toStdString(), signature.toStdString());
            }
            data = file.map(0, size);
            if (!data) {
                file.close();
                return false;
            }
            bool ret = m_walletImpl->verifySignedMessage(std::string(reinterpret_cast<const char*>(data), size), address.toStdString(), signature.toStdString());
            file.unmap(data);
            file.close();
            return ret;
        }
        catch (const std::exception &e) {
            if (data)
                file.unmap(data);
            file.close();
            return false;
        }
    }
    else {
        return m_walletImpl->verifySignedMessage(message.toStdString(), address.toStdString(), signature.toStdString());
    }
}

// #################### URI Parsing ####################

bool Wallet::parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const {
   std::string s_address, s_payment_id, s_tx_description, s_recipient_name, s_error;
   std::vector<std::string> s_unknown_parameters;
   bool res= m_walletImpl->parse_uri(uri.toStdString(), s_address, s_payment_id, amount, s_tx_description, s_recipient_name, s_unknown_parameters, s_error);
   if(res)
   {
       address = QString::fromStdString(s_address);
       payment_id = QString::fromStdString(s_payment_id);
       tx_description = QString::fromStdString(s_tx_description);
       recipient_name = QString::fromStdString(s_recipient_name);
       for( const auto &p : s_unknown_parameters )
           unknown_parameters.append(QString::fromStdString(p));
   }
   error = QString::fromStdString(s_error);
   return res;
}

QVariantMap Wallet::parse_uri_to_object(const QString &uri) const {
    QString address;
    QString payment_id;
    uint64_t amount = 0;
    QString tx_description;
    QString recipient_name;
    QVector<QString> unknown_parameters;
    QString error;

    QVariantMap result;
    if (this->parse_uri(uri, address, payment_id, amount, tx_description, recipient_name, unknown_parameters, error)) {
        result.insert("address", address);
        result.insert("payment_id", payment_id);
        result.insert("amount", amount > 0 ? QString::fromStdString(Monero::Wallet::displayAmount(amount)) : "");
        result.insert("tx_description", tx_description);
        result.insert("recipient_name", recipient_name);
    } else {
        result.insert("error", error);
    }

    return result;
}

QString Wallet::make_uri(const QString &address, quint64 &amount, const QString &description, const QString &recipient) const {
    std::string error;
    std::string uri = m_walletImpl->make_uri(address.toStdString(), "", amount, description.toStdString(), recipient.toStdString(), error);
    return QString::fromStdString(uri);
}

// #################### Misc / Unused ####################

quint64 Wallet::getBytesReceived() const {
    // TODO: this can segfault. Unclear why.
    try {
        return m_walletImpl->getBytesReceived();
    }
    catch (...) {
        return 0;
    }
}

quint64 Wallet::getBytesSent() const {
    return m_walletImpl->getBytesSent();
}

QString Wallet::getDaemonLogPath() const {
    return QString::fromStdString(m_walletImpl->getDefaultDataDir()) + "/bitmonero.log";
}

bool Wallet::setRingDatabase(const QString &path) {
    return m_walletImpl->setRingDatabase(path.toStdString());
}

void Wallet::setWalletCreationHeight(quint64 height) {
    m_walletImpl->setRefreshFromBlockHeight(height);
}

//! create a view only wallet
bool Wallet::createViewOnly(const QString &path, const QString &password) const {
    // Create path
    QDir d = QFileInfo(path).absoluteDir();
    d.mkpath(d.absolutePath());
    return m_walletImpl->createWatchOnly(path.toStdString(),password.toStdString(),m_walletImpl->getSeedLanguage());
}

bool Wallet::rescanSpent() {
    QMutexLocker locker(&m_asyncMutex);

    return m_walletImpl->rescanSpent();
}

void Wallet::setNewWallet() {
    m_newWallet = true;
}

Wallet::~Wallet()
{
    qDebug("~Wallet: Closing wallet");

    pauseRefresh();
    m_walletImpl->stop();

    m_scheduler.shutdownWaitForFinished();

    if (status() == Status_Critical || status() == Status_BadPassword) {
        qDebug("Not storing wallet cache");
    }
    else {
        bool success = m_walletImpl->store();
        success ? qDebug("Wallet cache stored successfully") : qDebug("Error storing wallet cache");
    }

    delete m_walletImpl;
    m_walletImpl = nullptr;
    qDebug("m_walletImpl deleted");
}
