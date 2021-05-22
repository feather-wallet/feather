// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "SubaddressAccount.h"
#include <QDebug>

SubaddressAccount::SubaddressAccount(Monero::SubaddressAccount *subaddressAccountImpl, QObject *parent)
  : QObject(parent), m_subaddressAccountImpl(subaddressAccountImpl)
{
    getAll();
}

void SubaddressAccount::getAll() const
{
    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);
        m_rows.clear();
        for (auto &row: m_subaddressAccountImpl->getAll()) {
            m_rows.append(row);
        }
    }

    emit refreshFinished();
}

bool SubaddressAccount::getRow(int index, std::function<void (Monero::SubaddressAccountRow &)> callback) const
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size())
    {
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

void SubaddressAccount::addRow(const QString &label) const
{
    m_subaddressAccountImpl->addRow(label.toStdString());
    getAll();
}

void SubaddressAccount::setLabel(quint32 accountIndex, const QString &label) const
{
    m_subaddressAccountImpl->setLabel(accountIndex, label.toStdString());
    getAll();
}

void SubaddressAccount::refresh() const
{
    m_subaddressAccountImpl->refresh();
    getAll();
}

quint64 SubaddressAccount::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.size();
}

Monero::SubaddressAccountRow* SubaddressAccount::row(int index) const
{
    return m_rows.value(index);
}