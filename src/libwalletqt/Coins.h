// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_COINS_H
#define FEATHER_COINS_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>

#include "Wallet.h"

namespace Monero {
    struct TransactionHistory;
}

namespace tools {
    class wallet2;
}

class CoinsInfo;

class Coins : public QObject
{
Q_OBJECT

public:
    bool coin(int index, std::function<void (CoinsInfo &)> callback);
    CoinsInfo * coin(int index);
    void refreshUnlocked();
    void freeze(QString &publicKey);
    void thaw(QString &publicKey);
    QVector<CoinsInfo*> coins_from_txid(const QString &txid);
    QVector<CoinsInfo*> coinsFromKeyImage(const QStringList &keyimages);
    void setDescription(const QString &publicKey, quint32 accountIndex, const QString &description);

    quint64 count() const;
    void clearRows();

public slots:
    void refresh();

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void coinFrozen() const;
    void coinThawed() const;
    void descriptionChanged() const;

private:
    explicit Coins(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent = nullptr);

private:
    friend class Wallet;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<CoinsInfo*> m_rows;

    mutable QReadWriteLock m_lock;
};

#endif //FEATHER_COINS_H
