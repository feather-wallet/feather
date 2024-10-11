// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "ConstructionInfo.h"

#include "Input.h"
#include "Transfer.h"
#include <wallet/api/wallet2_api.h>

quint64 ConstructionInfo::unlockTime() const {
    return m_unlockTime;
}

QSet<quint32> ConstructionInfo::subaddressIndices() const {
    return m_subaddressIndices;
}

QVector<QString> ConstructionInfo::subaddresses() const {
    return m_subaddresses;
}

quint64 ConstructionInfo::minMixinCount() const {
    return m_minMixinCount;
}

QList<Input *> ConstructionInfo::inputs() const {
    return m_inputs;
}

QList<Transfer *> ConstructionInfo::outputs() const {
    return m_outputs;
}

ConstructionInfo::ConstructionInfo(const Monero::TransactionConstructionInfo *pimpl, QObject *parent)
        : QObject(parent)
        , m_unlockTime(pimpl->unlockTime())
        , m_minMixinCount(pimpl->minMixinCount())
{
    for (auto const &i : pimpl->inputs())
    {
        Input *input = new Input(i.amount, QString::fromStdString(i.pubkey), this);
        m_inputs.append(input);
    }

    for (auto const &o : pimpl->outputs())
    {
        Transfer *output = new Transfer(o.amount, QString::fromStdString(o.address), this);
        m_outputs.append(output);
    }
    for (uint32_t i : pimpl->subaddressIndices())
    {
        m_subaddressIndices.insert(i);
    }
}