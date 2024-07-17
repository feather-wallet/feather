// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_MULTISIGMESSAGESTORE_H
#define FEATHER_MULTISIGMESSAGESTORE_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>

#include "Wallet.h"

#include "wallet/message_store.h"
#include "cryptonote_basic/blobdatatype.h"

namespace tools {
    class wallet2;
}

class MultisigMessage;
class TxProposal;

class MultisigMessageStore : public QObject
{
Q_OBJECT

public:
    struct SignerInfo {
        QString label;
        QString publicKey;
    };

    enum SetupMode {
        AUTOMATIC = 0,
        SEMI_AUTOMATIC,
        MANUAL
    };

    struct SetupKey {
        quint32 threshold;
        quint32 participants;
        QString service;
        SetupMode mode;
    };

    bool txProposal(int index, std::function<void (TxProposal &)> callback);
    quint64 txProposalCount() const;


    bool message(int index, std::function<void (MultisigMessage &)> callback);

    void receiveMessages();

    MultisigMessage * message(int index);
    void refresh(bool next = true);

    void next(bool forceSync = false, bool calledFromRefresh = false);

    bool signTx(quint32 id);
    bool deleteMessage(quint32 id);
    void deleteAllMessages();

    std::string exportMessage(quint32 id);

    void sendReadyMessages();

    QString errorString();

    void setServiceDetails(const QString &serviceUrl, const QString &serviceLogin);

    bool registerChannel(QString &channel, quint32 user_limit);

    bool prepareMultisig();
    bool makeMultisig(quint32 threshold, const std::vector<std::string> &kexMessages);
    bool exchangeMultisig(const std::vector<std::string> &kexMessages);
    bool exportMultisig();
    bool importMultisig(const std::vector<cryptonote::blobdata> info);
    bool signMultisigTx(const cryptonote::blobdata &data);
    bool submitMultisigTx(const cryptonote::blobdata &data);

    quint64 count() const;
    void clearRows();

    bool havePartiallySignedTxWaiting();

    SignerInfo getSignerInfo(quint32 index);

    bool processSyncData();
    bool sendPendingTransaction(quint32 message_id, quint32 cosigner);

    QString createSetupKey(quint32 threshold, quint32 signers, const QString &service, const QString &channel, SetupMode mode);
    bool checkSetupKey(const QString &setupKey, SetupKey &key);

    void init(const QString &setupKey, const QString &ownLabel);
    void setSigner(quint32 index, const QString& label, const QString& address);

    QVector<SignerInfo> getSignerInfo();

    QString getRecoveryInfo();

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void multisigWalletCreated(const QString &address);
    void signersUpdated() const;

    void askToSign(PendingTransaction *tx) const;
    void statusChanged(const QString &status, bool finished);

    void multisigInfoExported();
    void multisigInfoImported();

    void connectionError();

private:
    explicit MultisigMessageStore(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent = nullptr);

    void setErrorString(const QString &errorString);
    void clearStatus();

private:
    friend class Wallet;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<MultisigMessage*> m_rows;

    QList<TxProposal*> m_txProposals;
    QHash<QString, quint64> m_txProposalsIndex;

//    QList<TxProposal*> m_txProposals;
    QReadWriteLock lock{QReadWriteLock::RecursionMode::Recursive};

    quint32 m_sendToIndex = 1;
    QString m_status;
    QString m_errorString;

    mutable QReadWriteLock m_lock;
};

#endif //FEATHER_MULTISIGMESSAGESTORE_H
