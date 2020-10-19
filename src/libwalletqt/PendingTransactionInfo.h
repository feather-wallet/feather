// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

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
    Q_PROPERTY(quint64 fee READ fee)
    Q_PROPERTY(quint64 dust READ dust)
    Q_PROPERTY(bool dustAddedToFee READ dustAddedToFee)
    Q_PROPERTY(QString txKey READ txKey)
    Q_PROPERTY(quint64 unlockTime READ unlockTime)
    Q_PROPERTY(QSet<quint32> subaddressIndices READ subaddressIndices)
    Q_PROPERTY(QVector<QString> subaddresses READ subaddresses)
    Q_PROPERTY(quint64 minMixinCount READ minMixinCount)
    Q_PROPERTY(QList<Input*> inputs READ inputs)
    Q_PROPERTY(QList<Transfer*> outputs READ outputs)

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
