// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef SUBADDRESSACCOUNT_H
#define SUBADDRESSACCOUNT_H

#include <functional>

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QDateTime>

class SubaddressAccount : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void getAll() const;
    Q_INVOKABLE bool getRow(int index, std::function<void (Monero::SubaddressAccountRow &)> callback) const;
    Q_INVOKABLE void addRow(const QString &label) const;
    Q_INVOKABLE void setLabel(quint32 accountIndex, const QString &label) const;
    Q_INVOKABLE void refresh() const;
    quint64 count() const;
    Monero::SubaddressAccountRow* row(int index) const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;

public slots:

private:
    explicit SubaddressAccount(Monero::SubaddressAccount * subaddressAccountImpl, QObject *parent);
    friend class Wallet;
    mutable QReadWriteLock m_lock;
    Monero::SubaddressAccount * m_subaddressAccountImpl;
    mutable QList<Monero::SubaddressAccountRow*> m_rows;
};

#endif // SUBADDRESSACCOUNT_H
