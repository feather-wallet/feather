// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

#ifndef FEATHER_CONSTRUCTIONINFO_H
#define FEATHER_CONSTRUCTIONINFO_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <QSet>

class Input;
class Transfer;

class ConstructionInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint64 unlockTime READ unlockTime)
    Q_PROPERTY(QSet<quint32> subaddressIndices READ subaddressIndices)
    Q_PROPERTY(QVector<QString> subaddresses READ subaddresses)
    Q_PROPERTY(quint64 minMixinCount READ minMixinCount)
    Q_PROPERTY(QList<Input*> inputs READ inputs)
    Q_PROPERTY(QList<Transfer*> outputs READ outputs)

public:
    quint64 unlockTime() const;
    QSet<quint32> subaddressIndices() const;
    QVector<QString> subaddresses() const;
    quint64 minMixinCount() const;
    QList<Input*> inputs() const;
    QList<Transfer*> outputs() const;

private:
    explicit ConstructionInfo(const Monero::TransactionConstructionInfo *pimpl, QObject *parent = nullptr);

    friend class PendingTransactionInfo;
    friend class UnsignedTransaction;
    quint64 m_unlockTime;
    QSet<quint32> m_subaddressIndices;
    QVector<QString> m_subaddresses;
    quint64 m_minMixinCount;
    mutable QList<Input*> m_inputs;
    mutable QList<Transfer*> m_outputs;
};


#endif //FEATHER_CONSTRUCTIONINFO_H
