// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TRANSACTIONINFO_H
#define TRANSACTIONINFO_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <QDateTime>
#include <QSet>

class Transfer;
class Ring;

class TransactionInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Direction direction READ direction)
    Q_PROPERTY(bool isPending READ isPending)
    Q_PROPERTY(bool isFailed READ isFailed)
    Q_PROPERTY(bool isCoinbase READ isCoinbase)
    Q_PROPERTY(double amount READ amount)
    Q_PROPERTY(quint64 atomicAmount READ atomicAmount)
    Q_PROPERTY(QString displayAmount READ displayAmount)
    Q_PROPERTY(QString fee READ fee)
    Q_PROPERTY(quint64 blockHeight READ blockHeight)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(QSet<quint32> subaddrIndex READ subaddrIndex)
    Q_PROPERTY(quint32 subaddrAccount READ subaddrAccount)
    Q_PROPERTY(QString label READ label)
    Q_PROPERTY(quint64 confirmations READ confirmations)
    Q_PROPERTY(quint64 confirmationsRequired READ confirmationsRequired)
    Q_PROPERTY(quint64 unlockTime READ unlockTime)
    Q_PROPERTY(QString hash READ hash)
    Q_PROPERTY(QDateTime timestamp READ timestamp)
    Q_PROPERTY(QString date READ date)
    Q_PROPERTY(QString time READ time)
    Q_PROPERTY(QString paymentId READ paymentId)
    Q_PROPERTY(QString destinations_formatted READ destinations_formatted)
    Q_PROPERTY(QString rings_formatted READ rings_formatted)

public:
    enum Direction {
        Direction_In  =  Monero::TransactionInfo::Direction_In,
        Direction_Out =  Monero::TransactionInfo::Direction_Out,
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
    explicit TransactionInfo(const Monero::TransactionInfo *pimpl, QObject *parent = nullptr);
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

#endif // TRANSACTIONINFO_H
