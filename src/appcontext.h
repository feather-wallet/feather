// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_APPCONTEXT_H
#define FEATHER_APPCONTEXT_H

#include <QObject>
#include <QTimer>

#include "utils/os/whonix.h"
#include "utils/networking.h"
#include "utils/FeatherSeed.h"
#include "utils/daemonrpc.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/nodes.h"

#include "libwalletqt/WalletManager.h"
#include "PendingTransaction.h"

class AppContext : public QObject
{
Q_OBJECT

public:
    explicit AppContext(Wallet *wallet);

    Wallet *wallet;
    Nodes *nodes;

    bool donationSending = false;

    QString tmpTxDescription; // TODO: remove the need for this var

    NetworkType::Type networkType;
    PendingTransaction::Priority tx_priority = PendingTransaction::Priority::Priority_Low;

    // libwalletqt
    bool refreshed = false;

    void commitTransaction(PendingTransaction *tx);
    void syncStatusUpdated(quint64 height, quint64 target);
    void updateBalance();
    void refreshModels();

    void storeWallet();

    void stopTimers();

    void addCacheTransaction(const QString &txid, const QString &txHex) const;
    QString getCacheTransaction(const QString &txid) const;

public slots:
    void onCreateTransaction(const QString &address, quint64 amount, const QString &description, bool all);
    void onCreateTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description);
    void onCancelTransaction(PendingTransaction *tx, const QVector<QString> &address);
    void onSweepOutputs(const QVector<QString> &keyImages, QString address, bool churn, int outputs);
    void onCreateTransactionError(const QString &msg);
    void onOpenAliasResolve(const QString &openAlias);
    void onSetRestoreHeight(quint64 height);
    void onMultiBroadcast(PendingTransaction *tx);
    void onDeviceButtonRequest(quint64 code);
    void onDeviceButtonPressed();
    void onDeviceError(const QString &message);

    void onTorSettingsChanged(); // should not be here

private slots:
    void onMoneySpent(const QString &txId, quint64 amount);
    void onMoneyReceived(const QString &txId, quint64 amount);
    void onUnconfirmedMoneyReceived(const QString &txId, quint64 amount);
    void onWalletUpdate();
    void onWalletRefreshed(bool success, const QString &message);

    void onWalletNewBlock(quint64 blockheight, quint64 targetHeight);
    void onHeightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight);
    void onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address);
    void onTransactionCommitted(bool status, PendingTransaction *t, const QStringList& txid);

signals:
    void balanceUpdated(quint64 balance, quint64 spendable);
    void blockchainSync(int height, int target);
    void refreshSync(int height, int target);
    void synchronized();
    void walletRefreshed();
    void transactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);
    void createTransactionError(QString message);
    void createTransactionCancelled(const QVector<QString> &address, quint64 amount);
    void createTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address);
    void openAliasResolveError(const QString &msg);
    void openAliasResolved(const QString &address, const QString &openAlias);
    void setRestoreHeightError(const QString &msg);
    void customRestoreHeightSet(int height);
    void initiateTransaction();
    void endTransaction();
    void deviceButtonRequest(quint64 code);
    void deviceButtonPressed();
    void deviceError(const QString &message);

private:
    DaemonRpc *m_rpc;
    QTimer m_storeTimer;
};

#endif //FEATHER_APPCONTEXT_H
