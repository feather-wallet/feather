// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#ifndef FEATHER_PENDINGTRANSACTIONINFO_H
#define FEATHER_PENDINGTRANSACTIONINFO_H

#include <wallet/api/wallet2_api.h>
#include "ConstructionInfo.h"
#include <QObject>
#include <QSet>

class Input;
class Transfer;

class PendingTransactionInfo : public ConstructionInfo
{
    Q_OBJECT

public:
    quint64 fee() const;
    quint64 dust() const;
    bool dustAddedToFee() const;
    QString txKey() const;

private:
    explicit PendingTransactionInfo(const Monero::PendingTransactionInfo *pimpl, QObject *parent = nullptr);

    friend class PendingTransaction;
    quint64 m_fee;
    quint64 m_dust;
    bool m_dustAddedToFee;
    QString m_txKey;
};

#endif //FEATHER_PENDINGTRANSACTIONINFO_H
