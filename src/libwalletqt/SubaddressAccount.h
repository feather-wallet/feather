// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef SUBADDRESSACCOUNT_H
#define SUBADDRESSACCOUNT_H

#include <QObject>
#include <QList>

#include "rows/AccountRow.h"

namespace tools {
    class wallet2;
}

class SubaddressAccount : public QObject
{
    Q_OBJECT

public:
    const QList<AccountRow>& getRows();
    const AccountRow& row(int index) const;

    void addRow(const QString &label);
    void setLabel(quint32 accountIndex, const QString &label);
    qsizetype count() const;

    void refresh();
    void clearRows();

signals:
    void refreshStarted() const;
    void refreshFinished() const;

private:
    explicit SubaddressAccount(tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    tools::wallet2 *m_wallet2;
    QList<AccountRow> m_rows;
};

#endif // SUBADDRESSACCOUNT_H
