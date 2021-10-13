// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef PENDINGTRANSACTION_H
#define PENDINGTRANSACTION_H

#include <QObject>
#include <QList>
#include <QVariant>

#include <wallet/api/wallet2_api.h>
#include "PendingTransactionInfo.h"

class PendingTransaction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(quint64 amount READ amount)
    Q_PROPERTY(quint64 dust READ dust)
    Q_PROPERTY(quint64 fee READ fee)
    Q_PROPERTY(QStringList txid READ txid)
    Q_PROPERTY(quint64 txCount READ txCount)
    Q_PROPERTY(QList<QVariant> subaddrIndices READ subaddrIndices)

public:
    enum Status {
        Status_Ok       = Monero::PendingTransaction::Status_Ok,
        Status_Error    = Monero::PendingTransaction::Status_Error,
        Status_Critical = Monero::PendingTransaction::Status_Critical
    };
    Q_ENUM(Status)

    enum Priority {
        Priority_Low    = Monero::PendingTransaction::Priority_Low,
        Priority_Medium = Monero::PendingTransaction::Priority_Medium,
        Priority_High   = Monero::PendingTransaction::Priority_High
    };
    Q_ENUM(Priority)


    Status status() const;
    QString errorString() const;
    Q_INVOKABLE bool commit();
    bool saveToFile(const QString &fileName);
    quint64 amount() const;
    quint64 dust() const;
    quint64 fee() const;
    QStringList txid() const;
    quint64 txCount() const;
    QList<QVariant> subaddrIndices() const;
    QByteArray unsignedTxToBin() const;
    QString unsignedTxToBase64() const;
    QString signedTxToHex(int index) const;
    void refresh();

    PendingTransactionInfo * transaction(int index) const;

private:
    explicit PendingTransaction(Monero::PendingTransaction * pt, QObject *parent = nullptr);

private:
    friend class Wallet;
    Monero::PendingTransaction * m_pimpl;
    mutable QList<PendingTransactionInfo*> m_pending_tx_info;
};

#endif // PENDINGTRANSACTION_H
