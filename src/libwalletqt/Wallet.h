// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef WALLET_H
#define WALLET_H

#include <QElapsedTimer>
#include <QObject>
#include <QMutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>

#include "wallet/api/wallet2_api.h"
#include "utils/scheduler.h"
#include "PendingTransaction.h"
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

struct TxKeyResult {
    bool succes = false;
    bool good = false;
    QString amount;
    bool inPool;
    uint64_t confirmations;
    QString errorString;
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

class Wallet : public QObject, public PassphrasePrompter
{
Q_OBJECT

public:
    explicit Wallet(Monero::Wallet *w, QObject *parent = nullptr);
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

    // ##### Status #####
    //! returns last operation's status
    Status status() const;

    //! return connection status
    ConnectionStatus connectionStatus() const;

    //! returns true if wallet is currently synchronized
    bool isSynchronized() const;

    //! return true if wallet is connected to a node
    bool isConnected() const;

    //! returns last operation's error message
    QString errorString() const;

    //! returns network type of the wallet.
    NetworkType::Type nettype() const;

    //! returns if view only wallet
    bool viewOnly() const;

    //! return true if deterministic keys
    bool isDeterministic() const;

    QString walletName() const;
    
    // ##### Balance #####
    //! returns balance
    quint64 balance() const;
    quint64 balance(quint32 accountIndex) const;
    quint64 balanceAll() const;

    //! returns unlocked balance
    quint64 unlockedBalance() const;
    quint64 unlockedBalance(quint32 accountIndex) const;
    quint64 unlockedBalanceAll() const;
    
    quint64 viewOnlyBalance(quint32 accountIndex) const;

    void updateBalance();

    // ##### Subaddresses and Accounts #####
    //! returns wallet's public address
    QString address(quint32 accountIndex, quint32 addressIndex) const;

    //! returns the subaddress index of the address
    SubaddressIndex subaddressIndex(const QString &address) const;

    quint32 currentSubaddressAccount() const;
    void switchSubaddressAccount(quint32 accountIndex);
    void addSubaddressAccount(const QString& label);
    quint32 numSubaddressAccounts() const;
    quint32 numSubaddresses(quint32 accountIndex) const;
    void addSubaddress(const QString& label);
    QString getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const;
    void setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label);
    void deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId);
    QString getSubaddressLookahead() const;

    // ##### Seed #####

    //! returns mnemonic seed
    QString getSeed(const QString &seedOffset) const;

    qsizetype seedLength() const;

    //! returns seed language
    QString getSeedLanguage() const;

    //! set seed language
    void setSeedLanguage(const QString &lang);

    //! Get wallet keys
    QString getSecretViewKey() const {return QString::fromStdString(m_walletImpl->secretViewKey());}
    QString getPublicViewKey() const {return QString::fromStdString(m_walletImpl->publicViewKey());}
    QString getSecretSpendKey() const {return QString::fromStdString(m_walletImpl->secretSpendKey());}
    QString getPublicSpendKey() const {return QString::fromStdString(m_walletImpl->publicSpendKey());}

    // ##### Node connection #####

    void setOffline(bool offline) const;

    //! indicates if daemon is trusted
    void setTrustedDaemon(bool arg);

    //! indicates if ssl should be used to connect to daemon
    void setUseSSL(bool ssl);

    //! Set daemon rpc user/pass
    void setDaemonLogin(const QString &daemonUsername = "", const QString &daemonPassword = "");

    //! initializes wallet asynchronously
    void initAsync(const QString &daemonAddress,
                   bool trustedDaemon = false,
                   quint64 upperTransactionLimit = 0,
                   const QString &proxyAddress = "");

    // ##### Synchronization (Refresh) #####
    void startRefresh();
    void pauseRefresh();

    //! returns current wallet's block height
    //! (can be less than daemon's blockchain height when wallet sync in progress)
    quint64 blockChainHeight() const;

    //! returns daemon's blockchain height
    quint64 daemonBlockChainHeight() const;

    //! returns daemon's blockchain target height
    quint64 daemonBlockChainTargetHeight() const;

    void syncStatusUpdated(quint64 height, quint64 target);

    void refreshModels();

    // ##### Hardware wallet #####
    bool isHwBacked() const;
    bool isLedger() const;
    bool isTrezor() const;

    bool isDeviceConnected() const;

    //! attempt to reconnect to hw-device
    bool reconnectDevice();

    // Passphrase entry for hardware wallets
    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort=false);
    void onWalletPassphraseNeeded(bool on_device) override;

    // ##### Import / Export #####
    bool hasUnknownKeyImages() const;
    
    //! export/import key images
    bool exportKeyImages(const QString& path, bool all = false);
    bool exportKeyImagesToStr(std::string &keyImages, bool all = false);
    
    bool importKeyImages(const QString& path);
    bool importKeyImagesFromStr(const std::string &keyImages);

    //! export/import outputs
    bool exportOutputs(const QString& path, bool all = false);
    bool exportOutputsToStr(std::string& outputs, bool all);
    
    bool importOutputs(const QString& path);
    bool importOutputsFromStr(const std::string &outputs);

    //! import a transaction
    bool importTransaction(const QString& txid);

    // ##### Wallet cache #####
    //! saves wallet to the file by given path
    //! empty path stores in current location
    void store();
    void storeSafer();

    //! returns wallet cache file path
    QString cachePath() const;

    //! returns wallet keys file path
    QString keysPath() const;

    //! changes the password using existing parameters (path, seed, seed lang)
    bool setPassword(const QString &oldPassword, const QString &newPassword);

    //! verify wallet password
    bool verifyPassword(const QString &password);

    //! Namespace your cacheAttribute keys to avoid collisions
    bool cacheAttributeExists(const QString &key);
    bool setCacheAttribute(const QString &key, const QString &val);
    QString getCacheAttribute(const QString &key) const;

    void addCacheTransaction(const QString &txid, const QString &txHex);
    QString getCacheTransaction(const QString &txid) const;

    bool setUserNote(const QString &txid, const QString &note);
    QString getUserNote(const QString &txid) const;

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

    // ##### Transactions #####
    void setSelectedInputs(const QStringList &selected);

    void createTransaction(const QString &address, quint64 amount, const QString &description, bool all);
    void createTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description);
    void sweepOutputs(const QVector<QString> &keyImages, QString address, bool churn, int outputs);

    void commitTransaction(PendingTransaction *tx, const QString &description="");
    void onTransactionCommitted(bool success, PendingTransaction *tx, const QStringList& txid, const QMap<QString, QString> &txHexMap);

    //! deletes transaction and frees memory
    void disposeTransaction(PendingTransaction * t);

    //! deletes unsigned transaction and frees memory
    void disposeTransaction(UnsignedTransaction * t);

    // ##### Transaction import #####
    //! does wallet have txid
    bool haveTransaction(const QString &txid);

    //! Sign a transfer from file
    UnsignedTransaction * loadTxFile(const QString &fileName);
    UnsignedTransaction * loadUnsignedTransactionFromStr(const std::string &data);
    
    //! Load an unsigned transaction from a base64 encoded string
    UnsignedTransaction * loadTxFromBase64Str(const QString &unsigned_tx);

    //! Load a signed transaction from file
    PendingTransaction * loadSignedTxFile(const QString &fileName);
    PendingTransaction * loadSignedTxFromStr(const std::string &data);

    //! Submit a transfer from file
    bool submitTxFile(const QString &fileName) const;

    // ##### Models #####
    TransactionHistory* history() const;
    TransactionHistoryProxyModel* historyModel();
    TransactionHistoryModel* transactionHistoryModel() const;
    AddressBook* addressBook() const;
    AddressBookModel* addressBookModel() const;
    Subaddress* subaddress() const;
    SubaddressModel* subaddressModel() const;
    SubaddressAccount* subaddressAccount() const;
    SubaddressAccountModel* subaddressAccountModel() const;
    Coins* coins() const;
    CoinsModel* coinsModel() const;

    // ##### Transaction proofs #####

    QString getTxKey(const QString &txid) const;
    void getTxKeyAsync(const QString &txid, const std::function<void (QVariantMap)> &callback);

    TxKeyResult checkTxKey(const QString &txid, const QString &tx_key, const QString &address);
    TxProof getTxProof(const QString &txid, const QString &address, const QString &message) const;
    TxProofResult checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature);
    void checkTxProofAsync(const QString &txid, const QString &address, const QString &message, const QString &signature);
    TxProof getSpendProof(const QString &txid, const QString &message) const;
    QPair<bool, bool> checkSpendProof(const QString &txid, const QString &message, const QString &signature) const;
    void checkSpendProofAsync(const QString &txid, const QString &message, const QString &signature);

    // ##### Sign / Verify message #####
    //! signing a message
    QString signMessage(const QString &message, bool filename = false, const QString &address = "") const;

    //! verify a signed message
    bool verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename = false) const;

    // ##### URI Parsing #####
    bool parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const;
    QVariantMap parse_uri_to_object(const QString &uri) const;

    QString make_uri(const QString &address, quint64 &amount, const QString &description, const QString &recipient) const;

    // ##### Misc / Unused #####

    quint64 getBytesReceived() const;
    quint64 getBytesSent() const;

    QString getDaemonLogPath() const;

    bool setRingDatabase(const QString &path);

    quint64 getWalletCreationHeight() const {return m_walletImpl->getRefreshFromBlockHeight();}
    void setWalletCreationHeight(quint64 height);

    //! Rescan spent outputs
    bool rescanSpent();

    //! Indicates that the wallet is new
    void setNewWallet();

    //! create a view only wallet
    bool createViewOnly(const QString &path, const QString &password) const;

    PendingTransaction::Priority tx_priority = PendingTransaction::Priority::Priority_Low;

    QString tmpTxDescription; // TODO: remove the need for this var
    bool refreshedOnce = false;

    void onHeightsRefreshed(bool success, quint64 daemonHeight, quint64 targetHeight);

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
    void walletCreationHeightChanged();
    void deviceButtonRequest(quint64 buttonCode);
    void deviceButtonPressed();
    void deviceError(const QString &message);
    void walletPassphraseNeeded(bool onDevice);
    void transactionCommitted(bool status, PendingTransaction *t, const QStringList& txid, const QMap<QString, QString> &txHexMap);
    void deviceShowAddressShowed();
    void transactionProofVerified(TxProofResult result);
    void spendProofVerified(QPair<bool, bool> result);

    void connectionStatusChanged(int status) const;
    void currentSubaddressAccountChanged() const;

    void refreshSync(int height, int target);
    void blockchainSync(int height, int target);
    void synchronized();
    void balanceUpdated(quint64 balance, quint64 spendable);
    void keysCorrupted();

    void transactionCreated(PendingTransaction *tx, const QVector<QString> &address);

    void donationSent();
    void walletRefreshed();

    void initiateTransaction();

    void selectedInputsChanged(const QStringList &selectedInputs);

    void multiBroadcast(const QMap<QString, QString> &txHexMap);
    void heightsRefreshed(bool success, quint64 daemonHeight, quint64 targetHeight);

private:
    // ###### Status ######
    void setConnectionStatus(ConnectionStatus value);

    // ##### Synchronization (Refresh) #####
    void startRefreshThread();
    void onNewBlock(uint64_t height);
    void onUpdated();
    void onRefreshed(bool success, const QString &message);

    // ##### Transactions #####
    void onTransactionCreated(Monero::PendingTransaction *mtx, const QVector<QString> &address);

private:
    friend class WalletManager;
    friend class WalletListenerImpl;

    Monero::Wallet *m_walletImpl;

    TransactionHistory *m_history;
    TransactionHistoryModel *m_historyModel;
    TransactionHistoryProxyModel *m_historySortFilterModel;

    AddressBook *m_addressBook;
    AddressBookModel *m_addressBookModel;

    quint64 m_daemonBlockChainHeight;
    quint64 m_daemonBlockChainTargetHeight;

    ConnectionStatus m_connectionStatus;

    uint32_t m_currentSubaddressAccount;
    Subaddress *m_subaddress;
    SubaddressModel *m_subaddressModel;
    SubaddressAccount *m_subaddressAccount;
    SubaddressAccountModel *m_subaddressAccountModel;

    Coins *m_coins;
    CoinsModel *m_coinsModel;

    QMutex m_asyncMutex;
    QString m_daemonUsername;
    QString m_daemonPassword;

    QMutex m_proxyMutex;
    std::atomic<bool> m_refreshNow;
    std::atomic<bool> m_refreshEnabled;
    WalletListenerImpl *m_walletListener;
    FutureScheduler m_scheduler;

    bool m_useSSL;
    bool donationSending = false;
    bool m_newWallet = false;

    QTimer *m_storeTimer = nullptr;
    std::set<std::string> m_selectedInputs;
};



#endif // WALLET_H
