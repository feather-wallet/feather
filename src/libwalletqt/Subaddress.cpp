// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Subaddress.h"
#include <QDebug>

Subaddress::Subaddress(Monero::Subaddress *subaddressImpl, QObject *parent)
    : QObject(parent)
    , m_subaddressImpl(subaddressImpl)
    , m_unusedLookahead(0)
{
    getAll();
}

QString Subaddress::errorString() const
{
    return QString::fromStdString(m_subaddressImpl->errorString());
}

void Subaddress::getAll() const
{
    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        m_unusedLookahead = 0;

        m_rows.clear();
        for (auto &row: m_subaddressImpl->getAll()) {
            m_rows.append(row);

            if (row->isUsed())
                m_unusedLookahead = 0;
            else
                m_unusedLookahead += 1;
        }
    }

    emit refreshFinished();
}

bool Subaddress::getRow(int index, std::function<void (Monero::SubaddressRow &row)> callback) const
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size())
    {
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

bool Subaddress::addRow(quint32 accountIndex, const QString &label) const
{
    bool r = m_subaddressImpl->addRow(accountIndex, label.toStdString());

    if (r)
        getAll();

    return r;
}

bool Subaddress::setLabel(quint32 accountIndex, quint32 addressIndex, const QString &label) const
{
    bool r = m_subaddressImpl->setLabel(accountIndex, addressIndex, label.toStdString());
    if (r) {
        getAll();
        emit labelChanged();
    }
    return r;
}

bool Subaddress::refresh(quint32 accountIndex) const
{
    bool r = m_subaddressImpl->refresh(accountIndex);
    getAll();
    return r;
}

quint64 Subaddress::unusedLookahead() const
{
    QReadLocker locker(&m_lock);

    return m_unusedLookahead;
}

quint64 Subaddress::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.size();
}

Monero::SubaddressRow* Subaddress::row(int index) const
{
    return m_rows.value(index);
}