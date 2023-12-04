// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef PENDINGTRANSACTION_H
#define PENDINGTRANSACTION_H

#include <QObject>
#include <QList>

#include <wallet/api/wallet2_api.h>
#include "PendingTransactionInfo.h"

class PendingTransaction : public QObject
{
    Q_OBJECT

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


    Status status() const;
    QString errorString() const;
    const std::exception_ptr getException() const;
    bool commit();
    bool saveToFile(const QString &fileName);
    quint64 amount() const;
    quint64 dust() const;
    quint64 fee() const;
    QStringList txid() const;
    quint64 txCount() const;
    QList<QVariant> subaddrIndices() const;
    std::string unsignedTxToBin() const;
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
