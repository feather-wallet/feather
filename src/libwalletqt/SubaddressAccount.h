// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SUBADDRESSACCOUNT_H
#define FEATHER_SUBADDRESSACCOUNT_H

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
    void refresh();
    qsizetype count() const;

    const AccountRow& row(int index) const;
    const QList<AccountRow>& getRows();

    void addRow(const QString &label);
    void setLabel(quint32 accountIndex, const QString &label);

signals:
    void refreshStarted() const;
    void refreshFinished() const;

private:
    explicit SubaddressAccount(tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    tools::wallet2 *m_wallet2;
    QList<AccountRow> m_rows;
};

#endif // FEATHER_SUBADDRESSACCOUNT_H
