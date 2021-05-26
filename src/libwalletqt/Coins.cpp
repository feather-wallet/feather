// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "Coins.h"

#include <QDebug>

#include "CoinsInfo.h"

#include <QFile>


bool Coins::coin(int index, std::function<void (CoinsInfo &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_tinfo.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %d transactions in backend", __FUNCTION__, m_pimpl->count());
        return false;
    }

    callback(*m_tinfo.value(index));
    return true;
}

CoinsInfo* Coins::coin(int index)
{
    return m_tinfo.value(index);
}

void Coins::refresh(quint32 accountIndex)
{
    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        qDeleteAll(m_tinfo);
        m_tinfo.clear();

        m_pimpl->refresh();
        for (const auto i : m_pimpl->getAll()) {
            if (i->subaddrAccount() != accountIndex) {
                continue;
            }

            m_tinfo.append(new CoinsInfo(i, this));
        }
    }

    emit refreshFinished();
}

void Coins::refreshUnlocked()
{
    QWriteLocker locker(&m_lock);

    for (CoinsInfo* c : m_tinfo) {
        if (!c->unlocked()) {
            bool unlocked = m_pimpl->isTransferUnlocked(c->unlockTime(), c->blockHeight());
            c->setUnlocked(unlocked);
        }
    }
}

quint64 Coins::count() const
{
    QReadLocker locker(&m_lock);

    return m_tinfo.count();
}

void Coins::freeze(int index) const
{
    m_pimpl->setFrozen(index);
    emit coinFrozen();
}

void Coins::thaw(int index) const
{
    m_pimpl->thaw(index);
    emit coinThawed();
}

QVector<CoinsInfo*> Coins::coins_from_txid(const QString &txid)
{
    QVector<CoinsInfo*> coins;

    for (int i = 0; i < this->count(); i++) {
        CoinsInfo* coin = this->coin(i);
        if (coin->hash() == txid) {
            coins.append(coin);
        }
    }
    return coins;
}

Coins::Coins(Monero::Coins *pimpl, QObject *parent)
        : QObject(parent)
        , m_pimpl(pimpl)
{

}