// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_APPCONTEXT_H
#define FEATHER_APPCONTEXT_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QTimer>

#include "utils/tails.h"
#include "utils/prices.h"
#include "utils/networking.h"
#include "utils/tor.h"
#include "utils/xmrto.h"
#include "utils/wsclient.h"
#include "utils/txfiathistory.h"
#include "widgets/RedditPost.h"
#include "widgets/CCSEntry.h"
#include "utils/seeds.h"
#include "utils/nodes.h"

#include "libwalletqt/WalletManager.h"
#include "utils/keysfiles.h"
#include "PendingTransaction.h"

#define SUBADDRESS_LOOKAHEAD_MINOR 200


class AppContext : public QObject
{
Q_OBJECT

public:
    explicit AppContext(QCommandLineParser *cmdargs);
    ~AppContext();
    bool isTails = false;
    bool isWhonix = false;
    bool isDebug = false;
    const QString featherDonationAddress = "47ntfT2Z5384zku39pTM6hGcnLnvpRYW2Azm87GiAAH2bcTidtq278TL6HmwyL8yjMeERqGEBs3cqC8vvHPJd1cWQrGC65f";
    const int featherDonationAmount = 50;  // euro
    bool featherDonationSending = false;

    QCommandLineParser *cmdargs;

    QString coinName = "monero";
    bool isTorSocks = false;
    QString homeDir;
    QString accountName;
    QString configRoot;
    QString configDirectory;
    QString defaultWalletDir;
    QString defaultWalletDirRoot;
    QString tmpTxDescription;

    QString walletPath;
    QString walletPassword = "";
    NetworkType::Type networkType;

    QString applicationPath;

    static void createConfigDirectory(const QString &dir) ;

    QMap<QString, unsigned int> heights;
    QMap<NetworkType::Type, RestoreHeightLookup*> restoreHeights;
    const unsigned int kdfRounds = 1;
    PendingTransaction::Priority tx_priority = PendingTransaction::Priority::Priority_Low;
    quint32 tx_mixin = static_cast<const quint32 &>(10);
    static constexpr const double cdiv = 1e12;
    QString seedLanguage = "English";  // 14 word `monero-seed` only has English

    QNetworkAccessManager *network;
    QNetworkAccessManager *networkClearnet;
    QNetworkProxy *networkProxy;

    Tor *tor;
    WSClient *ws;
    XmrTo *XMRTo;
    Nodes *nodes;
    static Prices *prices;
    static WalletKeysFilesModel *wallets;
    static double balance;
    static QMap<QString, QString> txDescriptionCache;
    static TxFiatHistory *txFiatHistory;

    // libwalletqt
    unsigned int blockHeight = 0;
    bool refreshed = false;
    WalletManager *walletManager;
    Wallet *currentWallet = nullptr;
    void createWallet(FeatherSeed seed, const QString &path, const QString &password);
    void createWalletViewOnly(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight);
    void createWalletFinish(const QString &password);
    void syncStatusUpdated(quint64 height, quint64 target);
    void updateBalance();
    void initTor();
    void initRestoreHeights();
    void initWS();
    void donateBeg();
    void walletClose(bool emitClosedSignal = true);
    void storeWallet();
    void refreshModels();

public slots:
    void onOpenWallet(const QString& path, const QString &password);
    void onCreateTransaction(const QString &address, const double amount, const QString &description, bool all);
    void onCreateTransaction(XmrToOrder *order);
    void onCancelTransaction(PendingTransaction *tx, const QString &address);
    void onSweepOutput(const QString &keyImage, QString address, bool churn, int outputs) const;
    void onCreateTransactionError(const QString &msg);
    void onOpenAliasResolve(const QString &openAlias);
    void onSetRestoreHeight(unsigned int height);
    void onPreferredFiatCurrencyChanged(const QString &symbol);

private slots:
    void onWSNodes(const QJsonArray &nodes);
    void onWSMessage(const QJsonObject& msg);
    void onWSCCS(const QJsonArray &ccs_data);
    void onWSReddit(const QJsonArray& reddit_data);

    void onMoneySpent(const QString &txId, quint64 amount);
    void onMoneyReceived(const QString &txId, quint64 amount);
    void onUnconfirmedMoneyReceived(const QString &txId, quint64 amount);
    void onWalletUpdate();
    void onWalletRefreshed();
    void onWalletOpened(Wallet *wallet);
    void onWalletNewBlock(quint64 blockheight, quint64 targetHeight);
    void onHeightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight);
    void onTransactionCreated(PendingTransaction *tx, const QString &address, const QString &paymentId, quint32 mixin);
    void onTransactionCommitted(bool status, PendingTransaction *t, const QStringList& txid);
    void onConnectionStatusChanged(int status);

signals:
    void balanceUpdated(double balance, double unlocked, QString balance_str, QString unlocked_str);
    void blockchainSync(int height, int target);
    void refreshSync(int height, int target);
    void synchronized();
    void blockHeightWSUpdated(QMap<QString, unsigned int> heights);
    void walletSynchronized();
    void walletOpened();
    void walletClosed();
    void walletCreatedError(const QString &msg);
    void walletCreated(Wallet *wallet);
    void walletOpenedError(QString msg);
    void walletOpenPasswordNeeded(bool invalidPassword);
    void transactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);
    void createTransactionError(QString message);
    void createTransactionCancelled(QString address, double amount);
    void createTransactionSuccess(PendingTransaction *tx, const QString &address, const quint32 &mixin);
    void redditUpdated(QList<QSharedPointer<RedditPost>> &posts);
    void nodesUpdated(QList<QSharedPointer<FeatherNode>> &nodes);
    void ccsUpdated(QList<QSharedPointer<CCSEntry>> &entries);
    void nodeSourceChanged(NodeSource nodeSource);
    void setCustomNodes(QList<FeatherNode> nodes);
    void ccsEmpty();
    void openAliasResolveError(const QString &msg);
    void openAliasResolved(const QString &address, const QString &openAlias);
    void setRestoreHeightError(const QString &msg);
    void customRestoreHeightSet(unsigned int height);
    void closeApplication();
    void donationNag();
    void initiateTransaction();
    void endTransaction();
    void walletClosing();
    void setTitle(const QString &title); // set window title

private:
    void sorry();
    const unsigned int m_donationBoundary = 15;
    UtilsNetworking *m_utilsNetworkingNodes;
    QTimer *m_storeTimer = new QTimer(this);
    QUrl m_wsUrl = QUrl(QStringLiteral("ws://dtg2clrd6iand4mwp2x6nhbqd3nqbxlbiw65f6vkwmmutxy2sijsnjyd.onion/ws"));
};

#endif //FEATHER_APPCONTEXT_H
