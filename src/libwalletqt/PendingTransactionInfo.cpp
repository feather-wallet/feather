// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PendingTransactionInfo.h"
#include <wallet/api/wallet2_api.h>

quint64 PendingTransactionInfo::fee() const {
    return m_fee;
}

quint64 PendingTransactionInfo::dust() const {
    return m_dust;
}

bool PendingTransactionInfo::dustAddedToFee() const {
    return m_dustAddedToFee;
}

QString PendingTransactionInfo::txKey() const {
    return m_txKey;
}

PendingTransactionInfo::PendingTransactionInfo(const Monero::PendingTransactionInfo *pimpl, QObject *parent)
    : ConstructionInfo(pimpl->constructionData(), parent)
    , m_fee(pimpl->fee())
    , m_dust(pimpl->dust())
    , m_dustAddedToFee(pimpl->dustAddedToFee())
    , m_txKey(QString::fromStdString(pimpl->txKey()))
{

}