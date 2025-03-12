// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "ConstructionInfo.h"
#include <wallet/api/wallet2_api.h>

ConstructionInfo::ConstructionInfo(const Monero::TransactionConstructionInfo *pimpl)
        : unlockTime(pimpl->unlockTime())
        , minMixinCount(pimpl->minMixinCount())
{
    for (auto const &i : pimpl->inputs())
    {
        inputs.emplace_back(i.amount, QString::fromStdString(i.pubkey));
    }

    for (auto const &o : pimpl->outputs())
    {
        outputs.emplace_back(o.amount, QString::fromStdString(o.address));
    }
    for (uint32_t i : pimpl->subaddressIndices())
    {
        subaddressIndices.insert(i);
    }
}
