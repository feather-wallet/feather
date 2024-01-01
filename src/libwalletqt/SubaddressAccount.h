// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef SUBADDRESSACCOUNT_H
#define SUBADDRESSACCOUNT_H

#include <functional>

#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QDateTime>

#include "Wallet.h"
#include "rows/AccountRow.h"

namespace tools {
    class wallet2;
}

class SubaddressAccount : public QObject
{
    Q_OBJECT

public:
    void getAll() const;
    bool getRow(int index, std::function<void (AccountRow &row)> callback) const;
    void addRow(const QString &label);

    void setLabel(quint32 accountIndex, const QString &label);

    void refresh();

    qsizetype count() const;
    void clearRows();

    AccountRow* row(int index) const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;

private:
    explicit SubaddressAccount(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<AccountRow*> m_rows;
};

#endif // SUBADDRESSACCOUNT_H
