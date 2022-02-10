// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "TransactionInfo.h"
#include "libwalletqt/WalletManager.h"
#include "Transfer.h"
#include "Ring.h"

TransactionInfo::Direction TransactionInfo::direction() const
{
    return m_direction;
}

bool TransactionInfo::isPending() const
{
    return m_pending;
}

bool TransactionInfo::isFailed() const
{
    return m_failed;
}

bool TransactionInfo::isCoinbase() const
{
    return m_coinbase;
}

quint64 TransactionInfo::balanceDelta() const
{
    if (m_direction == Direction_In) {
        return m_amount;
    }
    else if (m_direction == Direction_Out) {
        return m_amount + m_fee;
    }
    return m_amount;
}

double TransactionInfo::amount() const
{
    // there's no unsigned uint64 for JS, so better use double
    return displayAmount().toDouble();
}

quint64 TransactionInfo::atomicAmount() const
{
    return m_amount;
}

QString TransactionInfo::displayAmount() const
{
    return WalletManager::displayAmount(m_amount);
}

quint64 TransactionInfo::atomicFee() const
{
    return m_fee;
}

QString TransactionInfo::fee() const
{
    if(m_fee == 0)
        return "";
    return WalletManager::displayAmount(m_fee);
}

quint64 TransactionInfo::blockHeight() const
{
    return m_blockHeight;
}

QString TransactionInfo::description() const
{
    return m_description;
}

QSet<quint32> TransactionInfo::subaddrIndex() const
{
    return m_subaddrIndex;
}

quint32 TransactionInfo::subaddrAccount() const
{
    return m_subaddrAccount;
}

QString TransactionInfo::label() const
{
    return m_label;
}

quint64 TransactionInfo::confirmations() const
{
    return m_confirmations;
}

quint64 TransactionInfo::confirmationsRequired() const
{
    return (m_blockHeight < m_unlockTime) ? m_unlockTime - m_blockHeight : 10;
}

quint64 TransactionInfo::unlockTime() const
{
    return m_unlockTime;
}

QString TransactionInfo::hash() const
{
    return m_hash;
}

QDateTime TransactionInfo::timestamp() const
{
    return m_timestamp;
}

QString TransactionInfo::date() const
{
    return timestamp().date().toString(Qt::ISODate);
}

QString TransactionInfo::time() const
{
    return timestamp().time().toString(Qt::ISODate);
}

QString TransactionInfo::paymentId() const
{
    return m_paymentId;
}

QString TransactionInfo::destinations_formatted() const
{
    QString destinations;
    for (auto const& t: m_transfers) {
        if (!destinations.isEmpty())
          destinations += "<br> ";
        destinations +=  WalletManager::displayAmount(t->amount()) + ": " + t->address();
    }
    return destinations;
}

QList<QString> TransactionInfo::destinations() const
{
    QList<QString> dests;
    for (auto const& t: m_transfers) {
        dests.append(t->address());
    }
    return dests;
}

QList<Transfer*> TransactionInfo::transfers() const {
    return m_transfers;
}

QString TransactionInfo::rings_formatted() const
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

TransactionInfo::TransactionInfo(const Monero::TransactionInfo *pimpl, QObject *parent)
    : QObject(parent)
    , m_amount(pimpl->amount())
    , m_blockHeight(pimpl->blockHeight())
    , m_description(QString::fromStdString(pimpl->description()))
    , m_confirmations(pimpl->confirmations())
    , m_direction(static_cast<Direction>(pimpl->direction()))
    , m_failed(pimpl->isFailed())
    , m_fee(pimpl->fee())
    , m_hash(QString::fromStdString(pimpl->hash()))
    , m_label(QString::fromStdString(pimpl->label()))
    , m_paymentId(QString::fromStdString(pimpl->paymentId()))
    , m_pending(pimpl->isPending())
    , m_subaddrAccount(pimpl->subaddrAccount())
    , m_timestamp(QDateTime::fromSecsSinceEpoch(pimpl->timestamp()))
    , m_unlockTime(pimpl->unlockTime())
    , m_coinbase(pimpl->isCoinbase())
{
    for (auto const &t: pimpl->transfers())
    {
        Transfer *transfer = new Transfer(t.amount, QString::fromStdString(t.address), this);
        m_transfers.append(transfer);
    }
    for (auto const &r: pimpl->rings())
    {
        Ring *ring = new Ring(QString::fromStdString(r.first), r.second, this);
        m_rings.append(ring);
    }
    for (uint32_t i : pimpl->subaddrIndex())
    {
        m_subaddrIndex.insert(i);
    }
}
