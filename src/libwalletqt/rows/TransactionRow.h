// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

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
    ~TransactionRow() override;

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
    qint64 balanceDelta() const;
    double amount() const;
    qint64 atomicAmount() const;
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
    QString hash() const;
    QDateTime timestamp() const;
    QString date() const;
    QString time() const;
    QString paymentId() const;
    QList<QString> destinations() const;
    QList<Transfer*> transfers() const;
    QString rings_formatted() const;
    bool hasPaymentId() const;

private:
    explicit TransactionRow(QObject *parent);

private:
    friend class TransactionHistory;
    QList<Transfer*> m_transfers;
    QList<Ring*> m_rings;
    qint64 m_amount; // Amount that was sent (to destinations) or received, excludes tx fee
    qint64 m_balanceDelta; // How much the total balance was mutated as a result of this tx (includes tx fee)
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
