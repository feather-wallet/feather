// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TRANSACTIONROW_H
#define FEATHER_TRANSACTIONROW_H

#include <QSet>
#include <QDateTime>

struct Ring
{
    QString keyImage;
    std::vector<uint64_t> ringMembers;

    explicit Ring(QString _keyImage, std::vector<uint64_t> _ringMembers)
        : keyImage(std::move(_keyImage))
        , ringMembers(std::move(_ringMembers)) {}
};
struct Transfer;

struct TransactionRow
{
    enum Direction {
        Direction_In  =  0,
        Direction_Out =  1,
        Direction_Both // invalid direction value, used for filtering
    };

    QList<Transfer> transfers;
    QList<Ring> rings;
    qint64 amount; // Amount that was sent (to destinations) or received, excludes tx fee
    qint64 balanceDelta; // How much the total balance was mutated as a result of this tx (includes tx fee)
    quint64 blockHeight;
    QString description;
    quint64 confirmations;
    Direction direction;
    QString hash;
    QString label;
    QString paymentId;
    quint32 subaddrAccount;
    QSet<quint32> subaddrIndex;
    QDateTime timestamp;
    quint64 unlockTime;
    bool failed;
    bool pending;
    bool coinbase;
    quint64 fee;

    QString displayFee() const;
    QString displayAmount() const;
    double amountDouble() const;
    quint64 confirmationsRequired() const;
    QString date() const;
    QString time() const;
    QList<QString> destinations() const;
    QString rings_formatted() const;
    bool hasPaymentId() const;

    explicit TransactionRow();
};

#endif //FEATHER_TRANSACTIONROW_H
