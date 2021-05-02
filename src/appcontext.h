// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_APPCONTEXT_H
#define FEATHER_APPCONTEXT_H

#include <QObject>
#include <QTimer>

#include "utils/tails.h"
#include "utils/whonix.h"
#include "utils/prices.h"
#include "utils/networking.h"
#include "utils/TorManager.h"
#include "utils/wsclient.h"
#include "utils/txfiathistory.h"
#include "utils/FeatherSeed.h"
#include "utils/daemonrpc.h"
#include "widgets/RedditPost.h"
#include "widgets/CCSEntry.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/nodes.h"

#include "libwalletqt/WalletManager.h"
#include "utils/keysfiles.h"
#include "PendingTransaction.h"

class AppContext : public QObject
{
Q_OBJECT

public:
    explicit AppContext(QCommandLineParser *cmdargs);
    ~AppContext() override;

    QCommandLineParser *cmdargs;

    bool isTails = false;
    bool isWhonix = false;

    bool donationSending = false;

    QString defaultWalletDir;
    QString tmpTxDescription;

    QString walletPath;
    QString walletPassword = "";
    NetworkType::Type networkType;

    PendingTransaction::Priority tx_priority = PendingTransaction::Priority::Priority_Low;
    QString seedLanguage = "English";  // 14 word `monero-seed` only has English

    Nodes *nodes; // TODO: move this to mainwindow (?)

    static WalletKeysFilesModel *wallets;
    static QMap<QString, QString> txCache;

    static void createConfigDirectory(const QString &dir);

    // libwalletqt
    bool refreshed = false;
    WalletManager *walletManager;
    Wallet *currentWallet = nullptr;
    void createWallet(FeatherSeed seed, const QString &path, const QString &password, const QString &seedOffset = "");
    void createWalletFromDevice(const QString &path, const QString &password, int restoreHeight);
    void createWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, bool deterministic = false);
    void commitTransaction(PendingTransaction *tx);
    void syncStatusUpdated(quint64 height, quint64 target);
    void updateBalance();
    void initTor();
    void initWS();
    void donateBeg();
    void refreshModels();

    // Closes the currently opened wallet
    void closeWallet(bool emitClosedSignal = true, bool storeWallet = false);
    void storeWallet();

public slots:
    void onOpenWallet(const QString& path, const QString &password);
    void onWalletCreated(Wallet * wallet);
    void onCreateTransaction(const QString &address, quint64 amount, const QString &description, bool all);
    void onCreateTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description);
    void onCancelTransaction(PendingTransaction *tx, const QVector<QString> &address);
    void onSweepOutput(const QString &keyImage, QString address, bool churn, int outputs) const;
    void onCreateTransactionError(const QString &msg);
    void onOpenAliasResolve(const QString &openAlias);
    void onSetRestoreHeight(quint64 height);
    void onPreferredFiatCurrencyChanged(const QString &symbol);
    void onAmountPrecisionChanged(int precision);
    void onMultiBroadcast(PendingTransaction *tx);
    void onDeviceButtonRequest(quint64 code);
    void onTorSettingsChanged();
    void onInitialNetworkConfigured();
    void onDeviceError(const QString &message);

private slots:
    void onMoneySpent(const QString &txId, quint64 amount);
    void onMoneyReceived(const QString &txId, quint64 amount);
    void onUnconfirmedMoneyReceived(const QString &txId, quint64 amount);
    void onWalletUpdate();
    void onWalletRefreshed(bool success, const QString &message);
    void onWalletOpened(Wallet *wallet);
    void onWalletNewBlock(quint64 blockheight, quint64 targetHeight);
    void onHeightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight);
    void onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address);
    void onTransactionCommitted(bool status, PendingTransaction *t, const QStringList& txid);

signals:
    // Emitted just before the wallet is closed
    void walletAboutToClose();

    // Emitted after a wallet has been closed
    void walletClosed();

    void balanceUpdated(quint64 balance, quint64 spendable);
    void blockchainSync(int height, int target);
    void refreshSync(int height, int target);
    void synchronized();
    void walletRefreshed();
    void walletOpened();
    void walletCreatedError(const QString &msg);
    void walletCreated(Wallet *wallet);
    void walletOpenedError(QString msg);
    void walletOpenPasswordNeeded(bool invalidPassword, QString path);
    void transactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);
    void createTransactionError(QString message);
    void createTransactionCancelled(const QVector<QString> &address, double amount);
    void createTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address);
    void openAliasResolveError(const QString &msg);
    void openAliasResolved(const QString &address, const QString &openAlias);
    void setRestoreHeightError(const QString &msg);
    void customRestoreHeightSet(int height);
    void closeApplication();
    void donationNag();
    void initiateTransaction();
    void endTransaction();
    void deviceButtonRequest(quint64 code);
    void updatesAvailable(const QJsonObject &updates);
    void deviceError(const QString &message);

private:
    QTimer m_storeTimer;
    bool m_openWalletTriedOnce = false;
};

#endif //FEATHER_APPCONTEXT_H
