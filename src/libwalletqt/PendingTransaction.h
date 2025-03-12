// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PENDINGTRANSACTION_H
#define FEATHER_PENDINGTRANSACTION_H

#include <QObject>
#include <QList>

#include "rows/PendingTransactionInfo.h"

namespace Monero {
    class PendingTransaction;
}

class PendingTransaction : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Status_Ok       = 0,
        Status_Error    = 1,
        Status_Critical = 2
    };
    Q_ENUM(Status)

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
    quint64 weight(int index) const;
    void refresh();

    const PendingTransactionInfo& transaction(int index) const;

private:
    explicit PendingTransaction(Monero::PendingTransaction * pt, QObject *parent = nullptr);

private:
    friend class Wallet;
    Monero::PendingTransaction * m_pimpl;
    mutable QList<PendingTransactionInfo> m_pending_tx_info;
};

#endif // FEATHER_PENDINGTRANSACTION_H
