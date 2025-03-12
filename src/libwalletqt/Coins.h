// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_COINS_H
#define FEATHER_COINS_H

#include <QObject>
#include <QList>
#include <QReadWriteLock>

namespace Monero {
    struct TransactionHistory;
}

namespace tools {
    class wallet2;
}

class CoinsInfo;
class Wallet;
class Coins : public QObject
{
Q_OBJECT

public:
    void refresh();
    void refreshUnlocked();
    quint64 count() const;

    const CoinsInfo& getRow(qsizetype i);
    const QList<CoinsInfo>& getRows();

    void setDescription(const QString &publicKey, quint32 accountIndex, const QString &description);
    void freeze(QStringList &publicKeys);
    void thaw(QStringList &publicKeys);
    quint64 sumAmounts(const QStringList &keyImages);

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void descriptionChanged() const;

private:
    explicit Coins(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent = nullptr);
    friend class Wallet;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<CoinsInfo> m_rows;
};

#endif //FEATHER_COINS_H
