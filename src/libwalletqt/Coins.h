// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_COINS_H
#define FEATHER_COINS_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>
#include <wallet/api/wallet2_api.h>

namespace Monero {
    struct TransactionHistory;
}

class CoinsInfo;

class Coins : public QObject
{
Q_OBJECT

public:
    bool coin(int index, std::function<void (CoinsInfo &)> callback);
    CoinsInfo * coin(int index);
    void refresh(quint32 accountIndex);
    void refreshUnlocked();
    void freeze(QString &publicKey) const;
    void thaw(QString &publicKey) const;
    QVector<CoinsInfo*> coins_from_txid(const QString &txid);
    QVector<CoinsInfo*> coinsFromKeyImage(const QStringList &keyimages);
    void setDescription(const QString &publicKey, quint32 accountIndex, const QString &description);

    quint64 count() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void coinFrozen() const;
    void coinThawed() const;
    void descriptionChanged() const;

private:
    explicit Coins(Monero::Coins * pimpl, QObject *parent = nullptr);

private:
    friend class Wallet;
    mutable QReadWriteLock m_lock;
    Monero::Coins * m_pimpl;
    mutable QList<CoinsInfo*> m_tinfo;
};

#endif //FEATHER_COINS_H
