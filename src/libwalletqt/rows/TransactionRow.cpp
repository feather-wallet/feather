// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "TransactionRow.h"
#include "WalletManager.h"
#include "Transfer.h"
#include "Ring.h"

TransactionRow::TransactionRow(QObject *parent)
        : QObject(parent)
        , m_direction(TransactionRow::Direction_Out)
        , m_pending(false)
        , m_failed(false)
        , m_coinbase(false)
        , m_amount(0)
        , m_balanceDelta(0)
        , m_fee(0)
        , m_blockHeight(0)
        , m_subaddrAccount(0)
        , m_confirmations(0)
        , m_unlockTime(0)
        , m_confirmationsRequired(0)
{
}

TransactionRow::Direction TransactionRow::direction() const
{
    return m_direction;
}

bool TransactionRow::isPending() const
{
    return m_pending;
}

bool TransactionRow::isFailed() const
{
    return m_failed;
}

bool TransactionRow::isCoinbase() const
{
    return m_coinbase;
}

qint64 TransactionRow::balanceDelta() const
{
    return m_balanceDelta;
}

double TransactionRow::amount() const
{
    // there's no unsigned uint64 for JS, so better use double
    return displayAmount().toDouble();
}

qint64 TransactionRow::atomicAmount() const
{
    return m_amount;
}

QString TransactionRow::displayAmount() const
{
    return WalletManager::displayAmount(m_amount);
}

quint64 TransactionRow::atomicFee() const
{
    return m_fee;
}

QString TransactionRow::fee() const
{
    if(m_fee == 0)
        return "";
    return WalletManager::displayAmount(m_fee);
}

quint64 TransactionRow::blockHeight() const
{
    return m_blockHeight;
}

QString TransactionRow::description() const
{
    return m_description;
}

QSet<quint32> TransactionRow::subaddrIndex() const
{
    return m_subaddrIndex;
}

quint32 TransactionRow::subaddrAccount() const
{
    return m_subaddrAccount;
}

QString TransactionRow::label() const
{
    return m_label;
}

quint64 TransactionRow::confirmations() const
{
    return m_confirmations;
}

quint64 TransactionRow::confirmationsRequired() const
{
    return (m_blockHeight < m_unlockTime) ? m_unlockTime - m_blockHeight : 10;
}

quint64 TransactionRow::unlockTime() const
{
    return m_unlockTime;
}

QString TransactionRow::hash() const
{
    return m_hash;
}

QDateTime TransactionRow::timestamp() const
{
    return m_timestamp;
}

QString TransactionRow::date() const
{
    return timestamp().date().toString(Qt::ISODate);
}

QString TransactionRow::time() const
{
    return timestamp().time().toString(Qt::ISODate);
}

QString TransactionRow::paymentId() const
{
    return m_paymentId;
}

QList<QString> TransactionRow::destinations() const
{
    QList<QString> dests;
    for (auto const& t: m_transfers) {
        dests.append(t->address());
    }
    return dests;
}

QList<Transfer*> TransactionRow::transfers() const {
    return m_transfers;
}

QString TransactionRow::rings_formatted() const
{
    QString rings;
    for (auto const& r: m_rings) {
        rings += r->keyImage() + ": \n";
        for (uint64_t m : r->ringMembers()){
            rings += QString::number(m) + " ";
        }
        rings += "\n\n";
    }
    return rings;
}

TransactionRow::~TransactionRow()
{
    qDeleteAll(m_transfers);
    qDeleteAll(m_rings);
}
