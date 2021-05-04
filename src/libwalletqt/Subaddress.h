// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef SUBADDRESS_H
#define SUBADDRESS_H

#include <functional>

#include <wallet/api/wallet2_api.h>
#include <QReadWriteLock>
#include <QObject>
#include <QList>
#include <QDateTime>

class Subaddress : public QObject
{
    Q_OBJECT
public:
    void getAll() const;
    bool getRow(int index, std::function<void (Monero::SubaddressRow &row)> callback) const;
    bool addRow(quint32 accountIndex, const QString &label) const;
    bool setLabel(quint32 accountIndex, quint32 addressIndex, const QString &label) const;
    void refresh(quint32 accountIndex) const;
    quint64 unusedLookahead() const;
    quint64 count() const;
    QString errorString() const;
    Monero::SubaddressRow* row(int index) const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void labelChanged() const;

public slots:

private:
    explicit Subaddress(Monero::Subaddress * subaddressImpl, QObject *parent);
    friend class Wallet;
    mutable QReadWriteLock m_lock;
    Monero::Subaddress * m_subaddressImpl;
    mutable QList<Monero::SubaddressRow*> m_rows;
    mutable quint64 m_unusedLookahead;
};

#endif // SUBADDRESS_H
