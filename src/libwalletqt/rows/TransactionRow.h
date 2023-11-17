// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TRANSACTIONROW_H
#define FEATHER_TRANSACTIONROW_H

class Transfer;
class Ring;

#include <QObject>
#include <QSet>
#include <QDateTime>

class TransactionRow : public QObject
{
    Q_OBJECT

public:
    enum Direction {
        Direction_In  =  0,
        Direction_Out =  1,
        Direction_Both // invalid direction value, used for filtering
    };

    Q_ENUM(Direction)

    Direction  direction() const;
    bool isPending() const;
    bool isFailed() const;
    bool isCoinbase() const;
    quint64 balanceDelta() const;
    double amount() const;
    quint64 atomicAmount() const;
    QString displayAmount() const;
    QString fee() const;
    quint64 atomicFee() const;
    quint64 blockHeight() const;
    QString description() const;
    QSet<quint32> subaddrIndex() const;
    quint32 subaddrAccount() const;
    QString label() const;
    quint64 confirmations() const;
    quint64 confirmationsRequired() const;
    quint64 unlockTime() const;
    //! transaction_id
    QString hash() const;
    QDateTime timestamp() const;
    QString date() const;
    QString time() const;
    QString paymentId() const;
    //! only applicable for output transactions
    //! used in tx details popup
    QList<QString> destinations() const;
    QString destinations_formatted() const;
    QList<Transfer*> transfers() const;
    QString rings_formatted() const;

private:
    explicit TransactionRow();

//    TransactionRow(const Monero::TransactionInfo *pimpl, QObject *parent = nullptr);

private:
    friend class TransactionHistory;
    mutable QList<Transfer*> m_transfers;
    mutable QList<Ring*> m_rings;
    quint64 m_amount;
    quint64 m_blockHeight;
    QString m_description;
    quint64 m_confirmations;
    quint64 m_confirmationsRequired;
    Direction m_direction;
    bool m_failed;
    quint64 m_fee;
    QString m_hash;
    QString m_label;
    QString m_paymentId;
    bool m_pending;
    quint32 m_subaddrAccount;
    QSet<quint32> m_subaddrIndex;
    QDateTime m_timestamp;
    quint64 m_unlockTime;
    bool m_coinbase;
};


#endif //FEATHER_TRANSACTIONROW_H
