// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TRANSACTIONHISTORY_H
#define TRANSACTIONHISTORY_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>

#include "rows/TransactionRow.h"
#include "Wallet.h"
#include "wallet/wallet2.h"

namespace Monero {
struct TransactionHistory;
}

class TransactionInfo;

class TransactionHistory : public QObject
{
    Q_OBJECT

public:
    bool transaction(int index, std::function<void (TransactionRow &)> callback);
    TransactionRow * transaction(const QString &id);
    TransactionRow* transaction(int index);
    void refresh();
    void setTxNote(const QString &txid, const QString &note);
    bool writeCSV(const QString &path);
    quint64 count() const;
    QDateTime firstDateTime() const;
    QDateTime lastDateTime() const;
    quint64 minutesToUnlock() const;
    bool locked() const;
    void clearRows();

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void firstDateTimeChanged() const;
    void lastDateTimeChanged() const;
    void txNoteChanged() const;

private:
    explicit TransactionHistory(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent = nullptr);

private:
    friend class Wallet;
    mutable QReadWriteLock m_lock;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<TransactionRow*> m_rows;

    mutable QDateTime   m_firstDateTime;
    mutable QDateTime   m_lastDateTime;
    mutable int m_minutesToUnlock;
    // history contains locked transfers
    mutable bool m_locked;

    quint32 lastAccountIndex = 0;
};

#endif // TRANSACTIONHISTORY_H
