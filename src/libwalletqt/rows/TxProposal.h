//
// Created by user on 3/20/24.
//

#ifndef FEATHER_TXPROPOSAL_H
#define FEATHER_TXPROPOSAL_H

#include <QObject>
#include <QDateTime>

class TxProposal : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Pending = 0,
        Expired,
        Cant_Sign,
        Signed,
        Completed,
        Frozen,
        Double_Spend
    };

    quint64 messageId;
    QDateTime timestamp;
    Status status;
    QStringList spendsOutputs;
    quint64 txCount;
    QString txId;
    quint64 numSignatures;
    QStringList signers;
    QString from;
    qint64 balanceDelta;
    QString prefixHash;

    QString errorString();

private:
    explicit TxProposal(QObject *parent);

private:
    friend class MultisigMessageStore;
};

#endif //FEATHER_TXPROPOSAL_H
