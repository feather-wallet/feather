// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "Wallet.h"

#include <chrono>
#include <thread>

#include "TransactionHistory.h"
#include "AddressBook.h"
#include "Subaddress.h"
#include "SubaddressAccount.h"
#include "Coins.h"
#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "model/AddressBookModel.h"
#include "model/SubaddressModel.h"
#include "model/SubaddressAccountModel.h"
#include "model/CoinsModel.h"

#include "utils/ScopeGuard.h"

namespace {
    static constexpr char ATTRIBUTE_SUBADDRESS_ACCOUNT[] = "feather.subaddress_account";
}

Wallet::Wallet(QObject * parent)
        : Wallet(nullptr, parent)
{
}

Wallet::ConnectionStatus Wallet::connectionStatus() const
{
    return m_connectionStatus;
}

QString Wallet::getSeed(const QString &seedOffset) const
{
    return QString::fromStdString(m_walletImpl->seed(seedOffset.toStdString()));
}

QString Wallet::getSeedLanguage() const
{
    return QString::fromStdString(m_walletImpl->getSeedLanguage());
}

void Wallet::setSeedLanguage(const QString &lang)
{
    m_walletImpl->setSeedLanguage(lang.toStdString());
}

Wallet::Status Wallet::status() const
{
    return static_cast<Status>(m_walletImpl->status());
}

NetworkType::Type Wallet::nettype() const
{
    return static_cast<NetworkType::Type>(m_walletImpl->nettype());
}

bool Wallet::disconnected() const
{
    return m_disconnected;
}

bool Wallet::refreshing() const
{
    return m_refreshing;
}

void Wallet::refreshingSet(bool value)
{
    if (m_refreshing.exchange(value) != value)
    {
        emit refreshingChanged();
    }
}

void Wallet::setConnectionStatus(ConnectionStatus value)
{
    if (m_connectionStatus == value)
    {
        return;
    }

    m_connectionStatus = value;
    emit connectionStatusChanged(m_connectionStatus);

    bool disconnected = m_connectionStatus == Wallet::ConnectionStatus_Connecting ||
                        m_connectionStatus == Wallet::ConnectionStatus_Disconnected;

    if (m_disconnected != disconnected)
    {
        m_disconnected = disconnected;
        emit disconnectedChanged();
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
}

QString Wallet::getProxyAddress() const
{
    QMutexLocker locker(&m_proxyMutex);
    return m_proxyAddress;
}

void Wallet::setProxyAddress(QString address)
{
    m_scheduler.run([this, address] {
        {
            QMutexLocker locker(&m_proxyMutex);

            if (!m_walletImpl->setProxy(address.toStdString()))
            {
                qCritical() << "failed to set proxy" << address;
            }

            m_proxyAddress = address;
        }
        emit proxyAddressChanged();
    });
}

bool Wallet::synchronized() const
{
    // Misleading: this will return true if wallet has synchronized at least once even if it isn't currently synchronized
    return m_walletImpl->synchronized();
}

bool Wallet::isSynchronized() const
{
    return connectionStatus() == ConnectionStatus_Synchronized;
}

bool Wallet::isConnected() const
{
    auto status = connectionStatus();
    return status == ConnectionStatus_Synchronizing || status == ConnectionStatus_Synchronized;
}

QString Wallet::errorString() const
{
    return QString::fromStdString(m_walletImpl->errorString());
}

bool Wallet::setPassword(const QString &password)
{
    return m_walletImpl->setPassword(password.toStdString());
}

QString Wallet::getPassword()
{
    return QString::fromStdString(m_walletImpl->getPassword());
}

QString Wallet::address(quint32 accountIndex, quint32 addressIndex) const
{
    return QString::fromStdString(m_walletImpl->address(accountIndex, addressIndex));
}

SubaddressIndex Wallet::subaddressIndex(const QString &address) const
{
    std::pair<uint32_t, uint32_t> i;
    if (!m_walletImpl->subaddressIndex(address.toStdString(), i)) {
        return SubaddressIndex(-1, -1);
    }
    return SubaddressIndex(i.first, i.second);
}

QString Wallet::cachePath() const
{
    return QDir::toNativeSeparators(QString::fromStdString(m_walletImpl->filename()));
}

QString Wallet::keysPath() const
{
    return QDir::toNativeSeparators(QString::fromStdString(m_walletImpl->keysFilename()));;
}

//void Wallet::storeAsync(const QJSValue &callback, const QString &path /* = "" */)
//{
//    const auto future = m_scheduler.run(
//            [this, path] {
//                QMutexLocker locker(&m_asyncMutex);
//
//                return QJSValueList({m_walletImpl->store(path.toStdString())});
//            },
//            callback);
//    if (!future.first)
//    {
//        QJSValue(callback).call(QJSValueList({false}));
//    }
//}

void Wallet::store(const QString &path)
{
    m_walletImpl->store(path.toStdString());
}

bool Wallet::init(const QString &daemonAddress, bool trustedDaemon, quint64 upperTransactionLimit, bool isRecovering, bool isRecoveringFromDevice, quint64 restoreHeight, const QString& proxyAddress)
{
    qDebug() << "init non async";
    if (isRecovering){
        qDebug() << "RESTORING";
        m_walletImpl->setRecoveringFromSeed(true);
    }
    if (isRecoveringFromDevice){
        qDebug() << "RESTORING FROM DEVICE";
        m_walletImpl->setRecoveringFromDevice(true);
    }
    if (isRecovering || isRecoveringFromDevice) {
        m_walletImpl->setRefreshFromBlockHeight(restoreHeight);
    }

    {
        QMutexLocker locker(&m_proxyMutex);

        if (!m_walletImpl->init(daemonAddress.toStdString(), upperTransactionLimit, m_daemonUsername.toStdString(), m_daemonPassword.toStdString(), m_useSSL, false, proxyAddress.toStdString()))
        {
            return false;
        }

        m_proxyAddress = proxyAddress;
    }
    emit proxyAddressChanged();

    setTrustedDaemon(trustedDaemon);
    return true;
}

void Wallet::setDaemonLogin(const QString &daemonUsername, const QString &daemonPassword)
{
    // store daemon login
    m_daemonUsername = daemonUsername;
    m_daemonPassword = daemonPassword;
}

void Wallet::initAsync(
        const QString &daemonAddress,
        bool trustedDaemon /* = false */,
        quint64 upperTransactionLimit /* = 0 */,
        bool isRecovering /* = false */,
        bool isRecoveringFromDevice /* = false */,
        quint64 restoreHeight /* = 0 */,
        const QString &proxyAddress /* = "" */)
{
    qDebug() << "initAsync: " + daemonAddress;
    const auto future = m_scheduler.run([this, daemonAddress, trustedDaemon, upperTransactionLimit, isRecovering, isRecoveringFromDevice, restoreHeight, proxyAddress] {
        bool success = init(daemonAddress, trustedDaemon, upperTransactionLimit, isRecovering, isRecoveringFromDevice, restoreHeight, proxyAddress);
        if (success)
        {
            emit walletCreationHeightChanged();
            qDebug() << "init async finished - starting refresh";
            startRefresh();
        }
    });
    if (future.first)
    {
        setConnectionStatus(Wallet::ConnectionStatus_Connecting);
    }
}

bool Wallet::setDaemon(const QString &daemonAddress)
{
    qDebug() << "setDaemon: " + daemonAddress;
    return m_walletImpl->setDaemon(daemonAddress.toStdString(), m_daemonUsername.toStdString(), m_daemonPassword.toStdString(), m_useSSL);
}

bool Wallet::isHwBacked() const
{
    return m_walletImpl->getDeviceType() != Monero::Wallet::Device_Software;
}

bool Wallet::isLedger() const
{
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Ledger;
}

bool Wallet::isTrezor() const
{
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Trezor;
}

bool Wallet::reconnectDevice()
{
    return m_walletImpl->reconnectDevice();
}

//! create a view only wallet
bool Wallet::createViewOnly(const QString &path, const QString &password) const
{
    // Create path
    QDir d = QFileInfo(path).absoluteDir();
    d.mkpath(d.absolutePath());
    return m_walletImpl->createWatchOnly(path.toStdString(),password.toStdString(),m_walletImpl->getSeedLanguage());
}

bool Wallet::connectToDaemon()
{
    return m_walletImpl->connectToDaemon();
}

void Wallet::setTrustedDaemon(bool arg)
{
    m_walletImpl->setTrustedDaemon(arg);
}

void Wallet::setUseSSL(bool ssl)
{
    m_useSSL = ssl;
}

bool Wallet::viewOnly() const
{
    return m_walletImpl->watchOnly();
}

bool Wallet::isDeterministic() const
{
    return m_walletImpl->isDeterministic();
}

quint64 Wallet::balance() const
{
    return balance(m_currentSubaddressAccount);
}

quint64 Wallet::balance(quint32 accountIndex) const
{
    return m_walletImpl->balance(accountIndex);
}

quint64 Wallet::balanceAll() const
{
    return m_walletImpl->balanceAll();
}

quint64 Wallet::unlockedBalance() const
{
    return unlockedBalance(m_currentSubaddressAccount);
}

quint64 Wallet::unlockedBalance(quint32 accountIndex) const
{
    return m_walletImpl->unlockedBalance(accountIndex);
}

quint64 Wallet::unlockedBalanceAll() const
{
    return m_walletImpl->unlockedBalanceAll();
}

quint32 Wallet::currentSubaddressAccount() const
{
    return m_currentSubaddressAccount;
}
void Wallet::switchSubaddressAccount(quint32 accountIndex)
{
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
        this->subaddressModel()->setCurrentSubaddressAcount(m_currentSubaddressAccount);
        this->coinsModel()->setCurrentSubaddressAccount(m_currentSubaddressAccount);
        emit currentSubaddressAccountChanged();
    }
}
void Wallet::addSubaddressAccount(const QString& label)
{
    m_walletImpl->addSubaddressAccount(label.toStdString());
    switchSubaddressAccount(numSubaddressAccounts() - 1);
}
quint32 Wallet::numSubaddressAccounts() const
{
    return m_walletImpl->numSubaddressAccounts();
}
quint32 Wallet::numSubaddresses(quint32 accountIndex) const
{
    return m_walletImpl->numSubaddresses(accountIndex);
}
void Wallet::addSubaddress(const QString& label)
{
    m_walletImpl->addSubaddress(currentSubaddressAccount(), label.toStdString());
}
QString Wallet::getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const
{
    return QString::fromStdString(m_walletImpl->getSubaddressLabel(accountIndex, addressIndex));
}
void Wallet::setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label)
{
    m_walletImpl->setSubaddressLabel(accountIndex, addressIndex, label.toStdString());
    emit currentSubaddressAccountChanged();
}
void Wallet::deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId)
{
    m_scheduler.run([this, accountIndex, addressIndex, paymentId] {
        m_walletImpl->deviceShowAddress(accountIndex, addressIndex, paymentId.toStdString());
        emit deviceShowAddressShowed();
    });
}

bool Wallet::refreshHeights()
{
    // daemonHeight and targetHeight will be 0 if call to get_info fails

    quint64 daemonHeight;
    QPair<bool, QFuture<void>> daemonHeightFuture = m_scheduler.run([this, &daemonHeight] {
        daemonHeight = m_walletImpl->daemonBlockChainHeight();;
    });
    if (!daemonHeightFuture.first)
    {
        return false;
    }
    // We must wait here for get_info to return, otherwise targetHeight will get cache value of 0
    daemonHeightFuture.second.waitForFinished();
    bool success = daemonHeight > 0;

    quint64 targetHeight = 0;
    if (success) {
        QPair<bool, QFuture<void>> targetHeightFuture = m_scheduler.run([this, &targetHeight] {
            targetHeight = m_walletImpl->daemonBlockChainTargetHeight();
        });
        if (!targetHeightFuture.first)
        {
            return false;
        }
        targetHeightFuture.second.waitForFinished();
    }

    m_daemonBlockChainHeight = daemonHeight;
    m_daemonBlockChainTargetHeight = targetHeight;

    success = (daemonHeight > 0 && targetHeight > 0);

    if (success) {
        quint64 walletHeight = blockChainHeight();
        emit heightRefreshed(walletHeight, daemonHeight, targetHeight);

        if (walletHeight < (daemonHeight - 1)) {
            setConnectionStatus(ConnectionStatus_Synchronizing);
        } else {
            setConnectionStatus(ConnectionStatus_Synchronized);
        }
    } else {
        setConnectionStatus(ConnectionStatus_Disconnected);
    }

    return success;
}

quint64 Wallet::blockChainHeight() const
{
    return m_walletImpl->blockChainHeight();
}

quint64 Wallet::daemonBlockChainHeight() const
{
    // Can not block UI
    return m_daemonBlockChainHeight;
}

quint64 Wallet::daemonBlockChainTargetHeight() const
{
    // Can not block UI
    return m_daemonBlockChainTargetHeight;
}

bool Wallet::exportKeyImages(const QString& path, bool all)
{
    return m_walletImpl->exportKeyImages(path.toStdString(), all);
}

bool Wallet::importKeyImages(const QString& path)
{
    return m_walletImpl->importKeyImages(path.toStdString());
}

bool Wallet::exportOutputs(const QString& path, bool all) {
    return m_walletImpl->exportOutputs(path.toStdString(), all);
}

bool Wallet::importOutputs(const QString& path) {
    return m_walletImpl->importOutputs(path.toStdString());
}

bool Wallet::importTransaction(const QString& txid, const QVector<quint64>& output_indeces, quint64 height, quint64 timestamp, bool miner_tx, bool pool, bool double_spend_seen) {
    std::vector<uint64_t> o_indeces;
    for (const auto &o : output_indeces) {
        o_indeces.push_back(o);
    }

    return m_walletImpl->importTransaction(
            txid.toStdString(),
            o_indeces,
            height,
            17, // todo: get actual block_version
            timestamp,
            miner_tx,
            pool,
            double_spend_seen);
}

QString Wallet::printBlockchain()
{
    return QString::fromStdString(m_walletImpl->printBlockchain());
}

QString Wallet::printTransfers()
{
    return QString::fromStdString(m_walletImpl->printTransfers());
}

QString Wallet::printPayments()
{
    return QString::fromStdString(m_walletImpl->printPayments());
}

QString Wallet::printUnconfirmedPayments()
{
    return QString::fromStdString(m_walletImpl->printUnconfirmedPayments());
}

QString Wallet::printConfirmedTransferDetails()
{
    return QString::fromStdString(m_walletImpl->printConfirmedTransferDetails());
}

QString Wallet::printUnconfirmedTransferDetails()
{
    return QString::fromStdString(m_walletImpl->printUnconfirmedTransferDetails());
}

QString Wallet::printPubKeys()
{
    return QString::fromStdString(m_walletImpl->printPubKeys());
}

QString Wallet::printTxNotes()
{
    return QString::fromStdString(m_walletImpl->printTxNotes());
}

QString Wallet::printSubaddresses()
{
    return QString::fromStdString(m_walletImpl->printSubaddresses());
}

QString Wallet::printSubaddressLabels()
{
    return QString::fromStdString(m_walletImpl->printSubaddressLabels());
}

QString Wallet::printAdditionalTxKeys()
{
    return QString::fromStdString(m_walletImpl->printAdditionalTxKeys());
}

QString Wallet::printAttributes()
{
    return QString::fromStdString(m_walletImpl->printAttributes());
}

QString Wallet::printKeyImages()
{
    return QString::fromStdString(m_walletImpl->printKeyImages());
}

QString Wallet::printAccountTags()
{
    return QString::fromStdString(m_walletImpl->printAccountTags());
}

QString Wallet::printTxKeys()
{
    return QString::fromStdString(m_walletImpl->printTxKeys());
}

QString Wallet::printAddressBook()
{
    return QString::fromStdString(m_walletImpl->printAddressBook());
}

QString Wallet::printScannedPoolTxs()
{
    return QString::fromStdString(m_walletImpl->printScannedPoolTxs());
}

bool Wallet::haveTransaction(const QString &txid)
{
    return m_walletImpl->haveTransaction(txid.toStdString());
}

void Wallet::startRefresh()
{
    m_refreshEnabled = true;
    m_refreshNow = true;
}

void Wallet::pauseRefresh()
{
    m_refreshEnabled = false;
}

PendingTransaction *Wallet::createTransaction(const QString &dst_addr, const QString &payment_id,
                                              quint64 amount, quint32 mixin_count,
                                              PendingTransaction::Priority priority)
{
//    pauseRefresh();

    std::set<uint32_t> subaddr_indices;
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransaction(
            dst_addr.toStdString(), payment_id.toStdString(), amount, mixin_count,
            static_cast<Monero::PendingTransaction::Priority>(priority), currentSubaddressAccount(), subaddr_indices);
    PendingTransaction * result = new PendingTransaction(ptImpl, nullptr);

//    startRefresh();
    return result;
}

void Wallet::createTransactionAsync(const QString &dst_addr, const QString &payment_id,
                                    quint64 amount, quint32 mixin_count,
                                    PendingTransaction::Priority priority)
{
    m_scheduler.run([this, dst_addr, payment_id, amount, mixin_count, priority] {
        PendingTransaction *tx = createTransaction(dst_addr, payment_id, amount, mixin_count, priority);
        QVector<QString> address {dst_addr};
        emit transactionCreated(tx, address);
    });
}

PendingTransaction* Wallet::createTransactionMultiDest(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                                       PendingTransaction::Priority priority)
{
//    pauseRefresh();

    std::vector<std::string> dests;
    for (auto &addr : dst_addr) {
        dests.push_back(addr.toStdString());
    }

    std::vector<uint64_t> amounts;
    for (auto &a : amount) {
        amounts.push_back(a);
    }

    // TODO: remove mixin count
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransactionMultDest(dests, "", amounts, 11, static_cast<Monero::PendingTransaction::Priority>(priority));
    PendingTransaction * result = new PendingTransaction(ptImpl);

//    startRefresh();
    return result;
}

void Wallet::createTransactionMultiDestAsync(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                             PendingTransaction::Priority priority)
{
    m_scheduler.run([this, dst_addr, amount, priority] {
        PendingTransaction *tx = createTransactionMultiDest(dst_addr, amount, priority);
        QVector<QString> addresses;
        for (auto &addr : dst_addr) {
            addresses.push_back(addr);
        }
        emit transactionCreated(tx, addresses);
    });
}

PendingTransaction *Wallet::createTransactionAll(const QString &dst_addr, const QString &payment_id,
                                                 quint32 mixin_count, PendingTransaction::Priority priority)
{
//    pauseRefresh();

    std::set<uint32_t> subaddr_indices;
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransaction(
            dst_addr.toStdString(), payment_id.toStdString(), Monero::optional<uint64_t>(), mixin_count,
            static_cast<Monero::PendingTransaction::Priority>(priority), currentSubaddressAccount(), subaddr_indices);
    PendingTransaction * result = new PendingTransaction(ptImpl, this);

//    startRefresh();
    return result;
}

void Wallet::createTransactionAllAsync(const QString &dst_addr, const QString &payment_id,
                                       quint32 mixin_count,
                                       PendingTransaction::Priority priority)
{
    m_scheduler.run([this, dst_addr, payment_id, mixin_count, priority] {
        PendingTransaction *tx = createTransactionAll(dst_addr, payment_id, mixin_count, priority);
        QVector<QString> address {dst_addr};
        emit transactionCreated(tx, address);
    });
}

PendingTransaction *Wallet::createTransactionSingle(const QString &key_image, const QString &dst_addr, const size_t outputs,
        PendingTransaction::Priority priority)
{
//    pauseRefresh();

    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransactionSingle(key_image.toStdString(), dst_addr.toStdString(),
            outputs, static_cast<Monero::PendingTransaction::Priority>(priority));
    PendingTransaction * result = new PendingTransaction(ptImpl, this);

//    startRefresh();
    return result;
}

void Wallet::createTransactionSingleAsync(const QString &key_image, const QString &dst_addr, const size_t outputs,
                                          PendingTransaction::Priority priority)
{
    m_scheduler.run([this, key_image, dst_addr, outputs, priority] {
        PendingTransaction *tx = createTransactionSingle(key_image, dst_addr, outputs, priority);
        QVector<QString> address {dst_addr};
        emit transactionCreated(tx, address);
    });
}

PendingTransaction *Wallet::createTransactionSelected(const QVector<QString> &key_images, const QString &dst_addr,
                                                      size_t outputs, PendingTransaction::Priority priority)
{
    std::vector<std::string> kis;
    for (const auto &key_image : key_images) {
        kis.push_back(key_image.toStdString());
    }
    Monero::PendingTransaction *ptImpl = m_walletImpl->createTransactionSelected(kis, dst_addr.toStdString(), outputs, static_cast<Monero::PendingTransaction::Priority>(priority));
    PendingTransaction *result = new PendingTransaction(ptImpl, this);

    return result;
}

void Wallet::createTransactionSelectedAsync(const QVector<QString> &key_images, const QString &dst_addr,
                                            size_t outputs, PendingTransaction::Priority priority)
{
    m_scheduler.run([this, key_images, dst_addr, outputs, priority] {
        PendingTransaction *tx = createTransactionSelected(key_images, dst_addr, outputs, priority);
        QVector<QString> address {dst_addr};
        emit transactionCreated(tx, address);
    });
}

PendingTransaction *Wallet::createSweepUnmixableTransaction()
{
//    pauseRefresh();

    Monero::PendingTransaction * ptImpl = m_walletImpl->createSweepUnmixableTransaction();
    PendingTransaction * result = new PendingTransaction(ptImpl, this);

//    startRefresh();
    return result;
}

void Wallet::createSweepUnmixableTransactionAsync()
{
    m_scheduler.run([this] {
        PendingTransaction *tx = createSweepUnmixableTransaction();
        QVector<QString> address {""};
        emit transactionCreated(tx, address);
    });
}

UnsignedTransaction * Wallet::loadTxFile(const QString &fileName)
{
    qDebug() << "Trying to sign " << fileName;
    Monero::UnsignedTransaction * ptImpl = m_walletImpl->loadUnsignedTx(fileName.toStdString());
    UnsignedTransaction * result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

UnsignedTransaction * Wallet::loadTxFromBase64Str(const QString &unsigned_tx)
{
    Monero::UnsignedTransaction * ptImpl = m_walletImpl->loadUnsignedTxFromBase64Str(unsigned_tx.toStdString());
    UnsignedTransaction * result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

PendingTransaction * Wallet::loadSignedTxFile(const QString &fileName)
{
    qDebug() << "Tying to load " << fileName;
    Monero::PendingTransaction * ptImpl = m_walletImpl->loadSignedTx(fileName.toStdString());
    PendingTransaction * result = new PendingTransaction(ptImpl, this);
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

bool Wallet::refresh(bool historyAndSubaddresses /* = true */)
{
    refreshingSet(true);
    const auto cleanup = sg::make_scope_guard([this]() noexcept {
        refreshingSet(false);
    });

    {
        QMutexLocker locker(&m_asyncMutex);

        bool result = m_walletImpl->refresh();
        if (historyAndSubaddresses)
        {
            m_history->refresh(currentSubaddressAccount());
            m_subaddress->refresh(currentSubaddressAccount());
            m_subaddressAccount->getAll();
        }

        return result;
    }
}

void Wallet::commitTransactionAsync(PendingTransaction *t, const QString &description)
{
    m_scheduler.run([this, t, description] {
        auto txIdList = t->txid();  // retrieve before commit
        bool success = t->commit();

        if (success && !description.isEmpty()) {
            for (const auto &txid : txIdList) {
                this->setUserNote(txid, description);
            }
        }

        emit transactionCommitted(success, t, txIdList);
    });
}

void Wallet::disposeTransaction(PendingTransaction *t)
{
    m_walletImpl->disposeTransaction(t->m_pimpl);
    delete t;
}

void Wallet::disposeTransaction(UnsignedTransaction *t)
{
    delete t;
}

//void Wallet::estimateTransactionFeeAsync(const QString &destination,
//                                         quint64 amount,
//                                         PendingTransaction::Priority priority,
//                                         const QJSValue &callback)
//{
//    m_scheduler.run([this, destination, amount, priority] {
//        const uint64_t fee = m_walletImpl->estimateTransactionFee(
//                {std::make_pair(destination.toStdString(), amount)},
//                static_cast<Monero::PendingTransaction::Priority>(priority));
//        return QJSValueList({QString::fromStdString(Monero::Wallet::displayAmount(fee))});
//    }, callback);
//}

TransactionHistory *Wallet::history() const
{
    return m_history;
}

TransactionHistoryProxyModel *Wallet::historyModel() const
{
    if (!m_historyModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_historyModel = new TransactionHistoryModel(w);
        m_historyModel->setTransactionHistory(this->history());
        m_historySortFilterModel = new TransactionHistoryProxyModel(w);
        m_historySortFilterModel->setSourceModel(m_historyModel);
        m_historySortFilterModel->setSortRole(TransactionHistoryModel::Date);
        m_historySortFilterModel->sort(0, Qt::DescendingOrder);
    }

    return m_historySortFilterModel;
}

TransactionHistoryModel *Wallet::transactionHistoryModel() const
{
    return m_historyModel;
}

AddressBook *Wallet::addressBook() const
{
    return m_addressBook;
}

AddressBookModel *Wallet::addressBookModel() const
{

    if (!m_addressBookModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_addressBookModel = new AddressBookModel(w,m_addressBook);
    }

    return m_addressBookModel;
}

Subaddress *Wallet::subaddress()
{
    return m_subaddress;
}

SubaddressModel *Wallet::subaddressModel()
{
    if (!m_subaddressModel) {
        m_subaddressModel = new SubaddressModel(this, m_subaddress);
    }
    return m_subaddressModel;
}

SubaddressAccount *Wallet::subaddressAccount() const
{
    return m_subaddressAccount;
}

SubaddressAccountModel *Wallet::subaddressAccountModel() const
{
    if (!m_subaddressAccountModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_subaddressAccountModel = new SubaddressAccountModel(w,m_subaddressAccount);
    }
    return m_subaddressAccountModel;
}

Coins *Wallet::coins() const
{
    return m_coins;
}

CoinsModel *Wallet::coinsModel() const
{
    if (!m_coinsModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_coinsModel = new CoinsModel(w, m_coins);
    }
    return m_coinsModel;
}

QString Wallet::generatePaymentId() const
{
    return QString::fromStdString(Monero::Wallet::genPaymentId());
}

QString Wallet::integratedAddress(const QString &paymentId) const
{
    return QString::fromStdString(m_walletImpl->integratedAddress(paymentId.toStdString()));
}

QString Wallet::getCacheAttribute(const QString &key) const {
    return QString::fromStdString(m_walletImpl->getCacheAttribute(key.toStdString()));
}

bool Wallet::setCacheAttribute(const QString &key, const QString &val)
{
    return m_walletImpl->setCacheAttribute(key.toStdString(), val.toStdString());
}

bool Wallet::setUserNote(const QString &txid, const QString &note)
{
    return m_walletImpl->setUserNote(txid.toStdString(), note.toStdString());
}

QString Wallet::getUserNote(const QString &txid) const
{
    return QString::fromStdString(m_walletImpl->getUserNote(txid.toStdString()));
}

QString Wallet::getTxKey(const QString &txid) const
{
    return QString::fromStdString(m_walletImpl->getTxKey(txid.toStdString()));
}

void Wallet::getTxKeyAsync(const QString &txid, const std::function<void (QVariantMap)> &callback)
{
    m_scheduler.run([this, txid] {
        QVariantMap map;
        map["tx_key"] = getTxKey(txid);
        return map;
    }, callback);
}

QString Wallet::checkTxKey(const QString &txid, const QString &tx_key, const QString &address)
{
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
    bool success = m_walletImpl->checkTxKey(txid.toStdString(), tx_key.toStdString(), address.toStdString(), received, in_pool, confirmations);
    std::string result = std::string(success ? "true" : "false") + "|" + QString::number(received).toStdString() + "|" + std::string(in_pool ? "true" : "false") + "|" + QString::number(confirmations).toStdString();
    return QString::fromStdString(result);
}

TxProof Wallet::getTxProof(const QString &txid, const QString &address, const QString &message) const
{
    std::string result = m_walletImpl->getTxProof(txid.toStdString(), address.toStdString(), message.toStdString());
    return TxProof(QString::fromStdString(result), QString::fromStdString(m_walletImpl->errorString()));
}

//void Wallet::getTxProofAsync(const QString &txid, const QString &address, const QString &message, const QJSValue &callback)
//{
//    m_scheduler.run([this, txid, address, message] {
//        return QJSValueList({txid, getTxProof(txid, address, message)});
//    }, callback);
//}

TxProofResult Wallet::checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature)
{
    bool good;
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
    bool success = m_walletImpl->checkTxProof(txid.toStdString(), address.toStdString(), message.toStdString(), signature.toStdString(), good, received, in_pool, confirmations);
    return {success, good, received, in_pool, confirmations};
}

void Wallet::checkTxProofAsync(const QString &txid, const QString &address, const QString &message,
                               const QString &signature) {
    m_scheduler.run([this, txid, address, message, signature] {
        auto result = this->checkTxProof(txid, address, message, signature);
        emit transactionProofVerified(result);
    });
}

Q_INVOKABLE TxProof Wallet::getSpendProof(const QString &txid, const QString &message) const
{
    std::string result = m_walletImpl->getSpendProof(txid.toStdString(), message.toStdString());
    return TxProof(QString::fromStdString(result), QString::fromStdString(m_walletImpl->errorString()));
}

//void Wallet::getSpendProofAsync(const QString &txid, const QString &message, const QJSValue &callback)
//{
//    m_scheduler.run([this, txid, message] {
//        return QJSValueList({txid, getSpendProof(txid, message)});
//    }, callback);
//}

Q_INVOKABLE QPair<bool, bool> Wallet::checkSpendProof(const QString &txid, const QString &message,
                                                      const QString &signature) const {
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

QString Wallet::signMessage(const QString &message, bool filename, const QString &address) const
{
  if (filename) {
    QFile file(message);
    uchar *data = NULL;

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

bool Wallet::verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename) const
{
  if (filename) {
    QFile file(message);
    uchar *data = NULL;

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

bool Wallet::parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const
{
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

QVariantMap Wallet::parse_uri_to_object(const QString &uri) const
{
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

QString Wallet::make_uri(const QString &address, quint64 &amount, const QString &description,
                         const QString &recipient) const
{
    std::string error;
    std::string uri = m_walletImpl->make_uri(address.toStdString(), "", amount, description.toStdString(), recipient.toStdString(), error);
    return QString::fromStdString(uri);
}

bool Wallet::rescanSpent()
{
    QMutexLocker locker(&m_asyncMutex);

    return m_walletImpl->rescanSpent();
}

bool Wallet::useForkRules(quint8 required_version, quint64 earlyBlocks) const
{
    if(m_connectionStatus == Wallet::ConnectionStatus_Disconnected)
        return false;
    try {
        return m_walletImpl->useForkRules(required_version,earlyBlocks);
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return false;
    }
}

void Wallet::setWalletCreationHeight(quint64 height)
{
    m_walletImpl->setRefreshFromBlockHeight(height);
    emit walletCreationHeightChanged();
}

QString Wallet::getDaemonLogPath() const
{
    return QString::fromStdString(m_walletImpl->getDefaultDataDir()) + "/bitmonero.log";
}

bool Wallet::blackballOutput(const QString &amount, const QString &offset)
{
    return m_walletImpl->blackballOutput(amount.toStdString(), offset.toStdString());
}

bool Wallet::blackballOutputs(const QList<QString> &pubkeys, bool add)
{
    std::vector<std::string> std_pubkeys;
    foreach (const QString &pubkey, pubkeys) {
        std_pubkeys.push_back(pubkey.toStdString());
    }
    return m_walletImpl->blackballOutputs(std_pubkeys, add);
}

bool Wallet::blackballOutputs(const QString &filename, bool add)
{
    QFile file(filename);

    try {
        if (!file.open(QIODevice::ReadOnly))
            return false;
        QList<QString> outputs;
        QTextStream in(&file);
        while (!in.atEnd()) {
            outputs.push_back(in.readLine());
        }
        file.close();
        return blackballOutputs(outputs, add);
    }
    catch (const std::exception &e) {
        file.close();
        return false;
    }
}

bool Wallet::unblackballOutput(const QString &amount, const QString &offset)
{
    return m_walletImpl->unblackballOutput(amount.toStdString(), offset.toStdString());
}

QString Wallet::getRing(const QString &key_image)
{
    std::vector<uint64_t> cring;
    if (!m_walletImpl->getRing(key_image.toStdString(), cring))
        return "";
    QString ring = "";
    for (uint64_t out: cring)
    {
        if (!ring.isEmpty())
            ring = ring + " ";
        QString s;
        s.setNum(out);
        ring = ring + s;
    }
    return ring;
}

QString Wallet::getRings(const QString &txid)
{
    std::vector<std::pair<std::string, std::vector<uint64_t>>> crings;
    if (!m_walletImpl->getRings(txid.toStdString(), crings))
        return "";
    QString ring = "";
    for (const auto &cring: crings)
    {
        if (!ring.isEmpty())
            ring = ring + "|";
        ring = ring + QString::fromStdString(cring.first) + " absolute";
        for (uint64_t out: cring.second)
        {
            ring = ring + " ";
            QString s;
            s.setNum(out);
            ring = ring + s;
        }
    }
    return ring;
}

bool Wallet::setRing(const QString &key_image, const QString &ring, bool relative)
{
    std::vector<uint64_t> cring;
    QStringList strOuts = ring.split(" ");
    foreach(QString str, strOuts)
    {
        uint64_t out;
        bool ok;
        out = str.toULong(&ok);
        if (ok)
            cring.push_back(out);
    }
    return m_walletImpl->setRing(key_image.toStdString(), cring, relative);
}

void Wallet::segregatePreForkOutputs(bool segregate)
{
    m_walletImpl->segregatePreForkOutputs(segregate);
}

void Wallet::segregationHeight(quint64 height)
{
    m_walletImpl->segregationHeight(height);
}

void Wallet::keyReuseMitigation2(bool mitigation)
{
    m_walletImpl->keyReuseMitigation2(mitigation);
}

void Wallet::onWalletPassphraseNeeded(bool on_device)
{
    emit this->walletPassphraseNeeded(on_device);
}

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

bool Wallet::isDeviceConnected() const {
    return m_walletImpl->isDeviceConnected();
}

void Wallet::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort)
{
    if (m_walletListener != nullptr)
    {
        m_walletListener->onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }
}

Wallet::Wallet(Monero::Wallet *w, QObject *parent)
        : QObject(parent)
        , m_walletImpl(w)
        , m_history(new TransactionHistory(m_walletImpl->history(), this))
        , m_historyModel(nullptr)
        , m_addressBook(new AddressBook(m_walletImpl->addressBook(), this))
        , m_addressBookModel(nullptr)
        , m_daemonBlockChainHeight(0)
        , m_daemonBlockChainTargetHeight(0)
        , m_connectionStatus(Wallet::ConnectionStatus_Disconnected)
        , m_disconnected(true)
        , m_currentSubaddressAccount(0)
        , m_subaddress(new Subaddress(m_walletImpl->subaddress(), this))
        , m_subaddressModel(nullptr)
        , m_subaddressAccount(new SubaddressAccount(m_walletImpl->subaddressAccount(), this))
        , m_subaddressAccountModel(nullptr)
        , m_coinsModel(nullptr)
        , m_refreshNow(false)
        , m_refreshEnabled(false)
        , m_refreshing(false)
        , m_scheduler(this)
        , m_useSSL(true)
        , m_coins(new Coins(m_walletImpl->coins(), this))
{
    m_walletListener = new WalletListenerImpl(this);
    m_walletImpl->setListener(m_walletListener);
    m_currentSubaddressAccount = getCacheAttribute(ATTRIBUTE_SUBADDRESS_ACCOUNT).toUInt();
    // start cache timers
    m_initialized = false;
    m_daemonUsername = "";
    m_daemonPassword = "";

    if (this->status() == Status_Ok) {
        startRefreshThread();
    }
}

Wallet::~Wallet()
{
    qDebug("~Wallet: Closing wallet");

    pauseRefresh();
    m_walletImpl->stop();

    m_scheduler.shutdownWaitForFinished();

    //Monero::WalletManagerFactory::getWalletManager()->closeWallet(m_walletImpl);
    if(status() == Status_Critical || status() == Status_BadPassword)
        qDebug("Not storing wallet cache");
    else if( m_walletImpl->store(""))
        qDebug("Wallet cache stored successfully");
    else
        qDebug("Error storing wallet cache");
    delete m_walletImpl;
    m_walletImpl = nullptr;
    delete m_walletListener;
    m_walletListener = NULL;
    qDebug("m_walletImpl deleted");
}

void Wallet::startRefreshThread()
{
    const auto future = m_scheduler.run([this] {
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
                    // Don't call refresh function if we don't have the daemon and target height
                    // We do this to prevent to UI from getting confused about the amount of blocks that are still remaining
                    bool haveHeights = refreshHeights();
                    if (haveHeights) {
                        refresh(false);
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

void Wallet::onRefreshed(bool success) {
    if (!success) {
        setConnectionStatus(ConnectionStatus_Disconnected);
    }
}