// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SubaddressAccount.h"
#include <wallet/wallet2.h>

SubaddressAccount::SubaddressAccount(tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet2(wallet2)
{
}

void SubaddressAccount::refresh()
{
    emit refreshStarted();

    m_rows.clear();

    for (uint32_t i = 0; i < m_wallet2->get_num_subaddress_accounts(); ++i)
    {
        m_rows.emplace_back(
            QString::fromStdString(m_wallet2->get_subaddress_as_str({i,0})),
            QString::fromStdString(m_wallet2->get_subaddress_label({i,0})),
            m_wallet2->balance(i, false),
            m_wallet2->unlocked_balance(i, false));
    }

    emit refreshFinished();
}

qsizetype SubaddressAccount::count() const
{
    return m_rows.length();
}

const AccountRow& SubaddressAccount::row(const int index) const
{
    if (index < 0 || index >= m_rows.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_rows[index];
}

const QList<AccountRow>& SubaddressAccount::getRows()
{
    return m_rows;
}

void SubaddressAccount::addRow(const QString &label)
{
    m_wallet2->add_subaddress_account(label.toStdString());
    refresh();
}

void SubaddressAccount::setLabel(quint32 accountIndex, const QString &label)
{
    m_wallet2->set_subaddress_label({accountIndex, 0}, label.toStdString());
    refresh();
}
