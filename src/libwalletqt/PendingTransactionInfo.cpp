// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PendingTransactionInfo.h"
#include <wallet/api/wallet2_api.h>

PendingTransactionInfo::PendingTransactionInfo(const Monero::PendingTransactionInfo *pimpl)
    : ConstructionInfo(pimpl->constructionData())
    , fee(pimpl->fee())
    , dust(pimpl->dust())
    , dustAddedToFee(pimpl->dustAddedToFee())
    , txKey(QString::fromStdString(pimpl->txKey()))
{

}
