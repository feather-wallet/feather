// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TRANSACTIONHISTORY_H
#define TRANSACTIONHISTORY_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>

namespace Monero {
struct TransactionHistory;
}

class TransactionInfo;

class TransactionHistory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QDateTime firstDateTime READ firstDateTime NOTIFY firstDateTimeChanged)
    Q_PROPERTY(QDateTime lastDateTime READ lastDateTime NOTIFY lastDateTimeChanged)
    Q_PROPERTY(int minutesToUnlock READ minutesToUnlock)
    Q_PROPERTY(bool locked READ locked)

public:
    Q_INVOKABLE bool transaction(int index, std::function<void (TransactionInfo &)> callback);
    Q_INVOKABLE TransactionInfo * transaction(const QString &id);
    TransactionInfo* transaction(int index);
    Q_INVOKABLE void refresh(quint32 accountIndex);
    Q_INVOKABLE void setTxNote(const QString &txid, const QString &note);
    Q_INVOKABLE bool writeCSV(const QString &path);
    quint64 count() const;
    QDateTime firstDateTime() const;
    QDateTime lastDateTime() const;
    quint64 minutesToUnlock() const;
    bool locked() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void firstDateTimeChanged() const;
    void lastDateTimeChanged() const;
    void txNoteChanged() const;

private:
    explicit TransactionHistory(Monero::TransactionHistory * pimpl, QObject *parent = nullptr);

private:
    friend class Wallet;
    mutable QReadWriteLock m_lock;
    Monero::TransactionHistory * m_pimpl;
    mutable QList<TransactionInfo*> m_tinfo;
    mutable QDateTime   m_firstDateTime;
    mutable QDateTime   m_lastDateTime;
    mutable int m_minutesToUnlock;
    // history contains locked transfers
    mutable bool m_locked;

    quint32 lastAccountIndex = 0;
};

#endif // TRANSACTIONHISTORY_H
