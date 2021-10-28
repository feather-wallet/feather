// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef WALLET_H
#define WALLET_H

#include <QElapsedTimer>
#include <QObject>
#include <QMutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>

#include "wallet/api/wallet2_api.h" // we need to have an access to the Monero::Wallet::Status enum here;
#include "utils/scheduler.h"
#include "PendingTransaction.h" // we need to have an access to the PendingTransaction::Priority enum here;
#include "UnsignedTransaction.h"
#include "utils/networktype.h"
#include "PassphraseHelper.h"
#include "WalletListenerImpl.h"

namespace Monero {
    struct Wallet; // forward declaration
}

struct TxProof {
    TxProof(QString proof, QString error = "")
        : proof(std::move(proof)), error(std::move(error)){}

    QString proof;
    QString error;
};

struct SubaddressIndex {
    SubaddressIndex(int major, int minor) {
        this->major = major;
        this->minor = minor;
    }

    bool isValid() const {
        return major >= 0 && minor >= 0;
    }

    bool isPrimary() const {
        return major == 0 && minor == 0;
    }

    int major;
    int minor;
};

class TransactionHistory;
class TransactionHistoryModel;
class TransactionHistoryProxyModel;
class AddressBook;
class AddressBookModel;
class Subaddress;
class SubaddressModel;
class SubaddressAccount;
class SubaddressAccountModel;
class Coins;
class CoinsModel;

struct TxProofResult {
    TxProofResult() {}
    TxProofResult(bool success, bool good, uint64_t received, bool in_pool, uint64_t confirmations)
      : success(success), good(good), received(received), in_pool(in_pool), confirmations(confirmations){}

    bool success;
    bool good;
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
};

class Wallet : public QObject, public PassprasePrompter
{
Q_OBJECT

public:
    explicit Wallet(QObject *parent = nullptr);
    explicit Wallet(Monero::Wallet *w, QObject * parent = nullptr);
    ~Wallet() override;

    enum Status {
        Status_Ok          = Monero::Wallet::Status_Ok,
        Status_Error       = Monero::Wallet::Status_Error,
        Status_Critical    = Monero::Wallet::Status_Critical,
        Status_BadPassword = Monero::Wallet::Status_BadPassword
    };

    Q_ENUM(Status)

    enum ConnectionStatus {
        ConnectionStatus_Disconnected    = Monero::Wallet::ConnectionStatus_Disconnected,
        ConnectionStatus_WrongVersion    = Monero::Wallet::ConnectionStatus_WrongVersion,
        ConnectionStatus_Connecting = 9,
        ConnectionStatus_Synchronizing = 10,
        ConnectionStatus_Synchronized = 11
    };

    Q_ENUM(ConnectionStatus)

    //! return connection status
    ConnectionStatus connectionStatus() const;

    //! returns mnemonic seed
    QString getSeed(const QString &seedOffset) const;

    //! returns seed language
    QString getSeedLanguage() const;

    //! set seed language
    void setSeedLanguage(const QString &lang);

    //! returns last operation's status
    Status status() const;

    //! returns network type of the wallet.
    NetworkType::Type nettype() const;

    //! returns true if wallet was ever synchronized
    bool synchronized() const;

    //! returns true if wallet is currently synchronized
    bool isSynchronized() const;

    //! return true if wallet is connected to a node
    bool isConnected() const;

    //! returns last operation's error message
    QString errorString() const;

    //! changes the password using existing parameters (path, seed, seed lang)
    bool setPassword(const QString &password);

    //! get current wallet password
    QString getPassword();

    //! returns wallet's public address
    QString address(quint32 accountIndex, quint32 addressIndex) const;

    //! returns the subaddress index of the address
    SubaddressIndex subaddressIndex(const QString &address) const;

    //! returns wallet cache file path
    QString cachePath() const;

    //! returns wallet keys file path
    QString keysPath() const;

    //! saves wallet to the file by given path
    //! empty path stores in current location
    void store(const QString &path = "");
   // void storeAsync(const QJSValue &callback, const QString &path = "");

    //! initializes wallet asynchronously
    void initAsync(
            const QString &daemonAddress,
            bool trustedDaemon = false,
            quint64 upperTransactionLimit = 0,
            bool isRecovering = false,
            bool isRecoveringFromDevice = false,
            quint64 restoreHeight = 0,
            const QString &proxyAddress = "");

    bool setDaemon(const QString &daemonAddress);

    // Set daemon rpc user/pass
    void setDaemonLogin(const QString &daemonUsername = "", const QString &daemonPassword = "");

    //! create a view only wallet
    bool createViewOnly(const QString &path, const QString &password) const;

    //! connects to daemon
    bool connectToDaemon();

    //! indicates if daemon is trusted
    void setTrustedDaemon(bool arg);

    //! indicates if ssl should be used to connect to daemon
    void setUseSSL(bool ssl);

    //! returns balance
    quint64 balance() const;
    quint64 balance(quint32 accountIndex) const;
    quint64 balanceAll() const;

    //! returns unlocked balance
    quint64 unlockedBalance() const;
    quint64 unlockedBalance(quint32 accountIndex) const;
    quint64 unlockedBalanceAll() const;

    //! account/address management
    quint32 currentSubaddressAccount() const;
    void switchSubaddressAccount(quint32 accountIndex);
    void addSubaddressAccount(const QString& label);
    quint32 numSubaddressAccounts() const;
    quint32 numSubaddresses(quint32 accountIndex) const;
    void addSubaddress(const QString& label);
    QString getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const;
    void setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label);
    void deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId);

    //! hw-device backed wallets
    bool isHwBacked() const;
    bool isLedger() const;
    bool isTrezor() const;

    //! attempt to reconnect to hw-device
    bool reconnectDevice();

    //! returns if view only wallet
    bool viewOnly() const;

    //! return true if deterministic keys
    bool isDeterministic() const;

    //! refresh daemon blockchain and target height
    bool refreshHeights();

    //! export/import key images
    bool exportKeyImages(const QString& path, bool all = false);
    bool importKeyImages(const QString& path);

    //! export/import outputs
    bool exportOutputs(const QString& path, bool all = false);
    bool importOutputs(const QString& path);

    //! import a transaction
    bool importTransaction(const QString& txid, const QVector<quint64>& output_indeces, quint64 height, quint64 timestamp, bool miner_tx, bool pool, bool double_spend_seen);

    QString printBlockchain();
    QString printTransfers();
    QString printPayments();
    QString printUnconfirmedPayments();
    QString printConfirmedTransferDetails();
    QString printUnconfirmedTransferDetails();
    QString printPubKeys();
    QString printTxNotes();
    QString printSubaddresses();
    QString printSubaddressLabels();
    QString printAdditionalTxKeys();
    QString printAttributes();
    QString printKeyImages();
    QString printAccountTags();
    QString printTxKeys();
    QString printAddressBook();
    QString printScannedPoolTxs();

    //! does wallet have txid
    bool haveTransaction(const QString &txid);

    //! refreshes the wallet
    bool refresh(bool historyAndSubaddresses = false);

    // pause/resume refresh
    void startRefresh();
    void pauseRefresh();

    //! returns current wallet's block height
    //! (can be less than daemon's blockchain height when wallet sync in progress)
    quint64 blockChainHeight() const;

    //! returns daemon's blockchain height
    quint64 daemonBlockChainHeight() const;

    //! returns daemon's blockchain target height
    quint64 daemonBlockChainTargetHeight() const;

    //! creates transaction
    PendingTransaction * createTransaction(const QString &dst_addr, const QString &payment_id,
                                                       quint64 amount, quint32 mixin_count,
                                                       PendingTransaction::Priority priority);

    //! creates async transaction
    void createTransactionAsync(const QString &dst_addr, const QString &payment_id,
                                            quint64 amount, quint32 mixin_count,
                                            PendingTransaction::Priority priority);

    //! creates multi-destination transaction
    PendingTransaction * createTransactionMultiDest(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                                                PendingTransaction::Priority priority);

    //! creates async multi-destination transaction
    void createTransactionMultiDestAsync(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                                     PendingTransaction::Priority priority);


    //! creates transaction with all outputs
    PendingTransaction * createTransactionAll(const QString &dst_addr, const QString &payment_id,
                                                          quint32 mixin_count, PendingTransaction::Priority priority);

    //! creates async transaction with all outputs
    void createTransactionAllAsync(const QString &dst_addr, const QString &payment_id,
                                               quint32 mixin_count, PendingTransaction::Priority priority);

    //! creates transaction with single input
    PendingTransaction * createTransactionSingle(const QString &key_image, const QString &dst_addr,
            size_t outputs, PendingTransaction::Priority priority);

    //! creates async transaction with single input
    void createTransactionSingleAsync(const QString &key_image, const QString &dst_addr,
            size_t outputs, PendingTransaction::Priority priority);

    //! creates transaction with selected inputs
    PendingTransaction * createTransactionSelected(const QVector<QString> &key_images, const QString &dst_addr,
                                                   size_t outputs, PendingTransaction::Priority priority);

    //! creates async transaction with selected inputs
    void createTransactionSelectedAsync(const QVector<QString> &key_images, const QString &dst_addr,
                                        size_t outputs, PendingTransaction::Priority priority);

    //! creates sweep unmixable transaction
    PendingTransaction * createSweepUnmixableTransaction();

    //! creates async sweep unmixable transaction
    void createSweepUnmixableTransactionAsync();

    //! Sign a transfer from file
    UnsignedTransaction * loadTxFile(const QString &fileName);

    //! Load an unsigned transaction from a base64 encoded string
    UnsignedTransaction * loadTxFromBase64Str(const QString &unsigned_tx);

    //! Load a signed transaction from file
    PendingTransaction * loadSignedTxFile(const QString &fileName);

    //! Submit a transfer from file
    bool submitTxFile(const QString &fileName) const;

    //! asynchronous transaction commit
    void commitTransactionAsync(PendingTransaction * t, const QString &description="");

    //! deletes transaction and frees memory
    void disposeTransaction(PendingTransaction * t);

    //! deletes unsigned transaction and frees memory
    void disposeTransaction(UnsignedTransaction * t);

//    void estimateTransactionFeeAsync(const QString &destination,
//                                                 quint64 amount,
//                                                 PendingTransaction::Priority priority,
//                                                 const QJSValue &callback);

    //! returns transaction history
    TransactionHistory * history() const;

    //! returns transaction history model
    TransactionHistoryProxyModel *historyModel() const;

    //! returns transaction history model (the real one)
    TransactionHistoryModel *transactionHistoryModel() const;

    //! returns Address book
    AddressBook *addressBook() const;

    //! returns adress book model
    AddressBookModel *addressBookModel() const;

    //! returns subaddress
    Subaddress *subaddress();

    //! returns subadress model
    SubaddressModel *subaddressModel();

    //! returns subaddress account
    SubaddressAccount *subaddressAccount() const;

    //! returns subadress account model
    SubaddressAccountModel *subaddressAccountModel() const;

    //! returns coins
    Coins *coins() const;

    //! return coins model
    CoinsModel *coinsModel() const;

    //! generate payment id
    QString generatePaymentId() const;

    //! integrated address
    QString integratedAddress(const QString &paymentId) const;

    //! signing a message
    QString signMessage(const QString &message, bool filename = false, const QString &address = "") const;

    //! verify a signed message
    bool verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename = false) const;

    //! Parse URI
    bool parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const;
    QVariantMap parse_uri_to_object(const QString &uri) const;

    QString make_uri(const QString &address, quint64 &amount, const QString &description, const QString &recipient) const;

    //! Namespace your cacheAttribute keys to avoid collisions
    bool setCacheAttribute(const QString &key, const QString &val);
    QString getCacheAttribute(const QString &key) const;

    bool setUserNote(const QString &txid, const QString &note);
    QString getUserNote(const QString &txid) const;
    QString getTxKey(const QString &txid) const;
    void getTxKeyAsync(const QString &txid, const std::function<void (QVariantMap)> &callback);

    QString checkTxKey(const QString &txid, const QString &tx_key, const QString &address);
    TxProof getTxProof(const QString &txid, const QString &address, const QString &message) const;
   // void getTxProofAsync(const QString &txid, const QString &address, const QString &message, const QJSValue &callback);
    //QString checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature);
    TxProofResult checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature);
    void checkTxProofAsync(const QString &txid, const QString &address, const QString &message, const QString &signature);
    TxProof getSpendProof(const QString &txid, const QString &message) const;
   // void getSpendProofAsync(const QString &txid, const QString &message, const QJSValue &callback);
    QPair<bool, bool> checkSpendProof(const QString &txid, const QString &message, const QString &signature) const;
    void checkSpendProofAsync(const QString &txid, const QString &message, const QString &signature);
    // Rescan spent outputs
    bool rescanSpent();

    // check if fork rules should be used
    bool useForkRules(quint8 version, quint64 earlyBlocks = 0) const;

    //! Get wallet keys
    QString getSecretViewKey() const {return QString::fromStdString(m_walletImpl->secretViewKey());}
    QString getPublicViewKey() const {return QString::fromStdString(m_walletImpl->publicViewKey());}
    QString getSecretSpendKey() const {return QString::fromStdString(m_walletImpl->secretSpendKey());}
    QString getPublicSpendKey() const {return QString::fromStdString(m_walletImpl->publicSpendKey());}

    quint64 getWalletCreationHeight() const {return m_walletImpl->getRefreshFromBlockHeight();}
    void setWalletCreationHeight(quint64 height);

    QString getDaemonLogPath() const;

    // Blackalled outputs
    bool blackballOutput(const QString &amount, const QString &offset);
    bool blackballOutputs(const QList<QString> &outputs, bool add);
    bool blackballOutputs(const QString &filename, bool add);
    bool unblackballOutput(const QString &amount, const QString &offset);

    // Rings
    QString getRing(const QString &key_image);
    QString getRings(const QString &txid);
    bool setRing(const QString &key_image, const QString &ring, bool relative);

    // key reuse mitigation options
    void segregatePreForkOutputs(bool segregate);
    void segregationHeight(quint64 height);
    void keyReuseMitigation2(bool mitigation);

    // Passphrase entry for hardware wallets
    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort=false);
    void onWalletPassphraseNeeded(bool on_device) override;

    quint64 getBytesReceived() const;
    quint64 getBytesSent() const;

    bool isDeviceConnected() const;

    // TODO: setListenter() when it implemented in API
signals:
    // emitted on every event happened with wallet
    // (money sent/received, new block)
    void updated();

    // emitted when refresh process finished (could take a long time)
    // signalling only after we
    void refreshed(bool success, const QString &message);

    void moneySpent(const QString &txId, quint64 amount);
    void moneyReceived(const QString &txId, quint64 amount);
    void unconfirmedMoneyReceived(const QString &txId, quint64 amount);
    void newBlock(quint64 height, quint64 targetHeight);
    void addressBookChanged() const;
    void historyModelChanged() const;
    void walletCreationHeightChanged();
    void deviceButtonRequest(quint64 buttonCode);
    void deviceButtonPressed();
    void deviceError(const QString &message);
    void walletPassphraseNeeded(bool onDevice);
    void transactionCommitted(bool status, PendingTransaction *t, const QStringList& txid);
    void heightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight) const;
    void deviceShowAddressShowed();
    void transactionProofVerified(TxProofResult result);
    void spendProofVerified(QPair<bool, bool> result);

    // emitted when transaction is created async
    void transactionCreated(PendingTransaction * transaction, QVector<QString> address);

    void connectionStatusChanged(int status) const;
    void currentSubaddressAccountChanged() const;
    void disconnectedChanged() const;
    void proxyAddressChanged() const;
    void refreshingChanged() const;

private:
    //! initializes wallet
    bool init(
            const QString &daemonAddress,
            bool trustedDaemon,
            quint64 upperTransactionLimit,
            bool isRecovering,
            bool isRecoveringFromDevice,
            quint64 restoreHeight,
            const QString& proxyAddress);

    bool disconnected() const;
    bool refreshing() const;
    void refreshingSet(bool value);
    void onRefreshed(bool success);

    void setConnectionStatus(ConnectionStatus value);
    QString getProxyAddress() const;
    void setProxyAddress(QString address);
    void startRefreshThread();

    void onNewBlock(uint64_t height);

private:
    friend class WalletManager;
    friend class WalletListenerImpl;
    //! libwallet's
    Monero::Wallet * m_walletImpl;
    // history lifetime managed by wallet;
    TransactionHistory * m_history;
    // Used for UI history view
    mutable TransactionHistoryModel * m_historyModel;
    mutable TransactionHistoryProxyModel * m_historySortFilterModel;
    QString m_paymentId;
    AddressBook * m_addressBook;
    mutable AddressBookModel * m_addressBookModel;
    mutable quint64 m_daemonBlockChainHeight;
    mutable quint64 m_daemonBlockChainTargetHeight;

    mutable ConnectionStatus m_connectionStatus;

    bool m_disconnected;
    mutable bool m_initialized;
    uint32_t m_currentSubaddressAccount;
    Subaddress * m_subaddress;
    mutable SubaddressModel * m_subaddressModel;
    SubaddressAccount * m_subaddressAccount;
    mutable SubaddressAccountModel * m_subaddressAccountModel;
    Coins * m_coins;
    mutable CoinsModel * m_coinsModel;
    QMutex m_asyncMutex;
    QString m_daemonUsername;
    QString m_daemonPassword;
    QString m_proxyAddress;
    mutable QMutex m_proxyMutex;
    std::atomic<bool> m_refreshNow;
    std::atomic<bool> m_refreshEnabled;
    std::atomic<bool> m_refreshing;
    WalletListenerImpl *m_walletListener;
    FutureScheduler m_scheduler;
    int m_connectionTimeout = 30;
    bool m_useSSL;
};



#endif // WALLET_H
