// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TRANSACTIONHISTORY_H
#define FEATHER_TRANSACTIONHISTORY_H

#include <QReadWriteLock>

#include "rows/TransactionRow.h"

namespace tools {
    class wallet2;
}

namespace Monero {
struct TransactionHistory;
}

class TransactionInfo;
class Wallet;
class TransactionHistory : public QObject
{
    Q_OBJECT

public:
    void refresh();
    quint64 count() const;

    const TransactionRow& transaction(int index);
    const QList<TransactionRow>& getRows();

    void setTxNote(const QString &txid, const QString &note);
    bool locked() const;

    QString importLabelsFromCSV(const QString &fileName);

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
    QList<TransactionRow> m_rows;

    mutable QDateTime   m_firstDateTime;
    mutable QDateTime   m_lastDateTime;
    mutable int m_minutesToUnlock;
    // history contains locked transfers
    mutable bool m_locked;

    quint32 lastAccountIndex = 0;
};

#endif // FEATHER_TRANSACTIONHISTORY_H
