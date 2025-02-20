// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SubaddressAccount.h"
#include <wallet/wallet2.h>

SubaddressAccount::SubaddressAccount(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet(wallet)
    , m_wallet2(wallet2)
{
}

bool SubaddressAccount::getRow(int index, std::function<void (AccountRow &row)> callback) const
{
    if (index < 0 || index >= m_rows.size())
    {
        return false;
    }

    callback(*m_rows.value(index));
    return true;
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

void SubaddressAccount::refresh()
{
    emit refreshStarted();

    this->clearRows();
    for (uint32_t i = 0; i < m_wallet2->get_num_subaddress_accounts(); ++i)
    {
        auto *row = new AccountRow{this,
                                   i,
                                   QString::fromStdString(m_wallet2->get_subaddress_as_str({i,0})),
                                   QString::fromStdString(m_wallet2->get_subaddress_label({i,0})),
                                   m_wallet2->balance(i, false),
                                   m_wallet2->unlocked_balance(i, false)};

        m_rows.append(row);
    }

    emit refreshFinished();
}

qsizetype SubaddressAccount::count() const
{
    return m_rows.length();
}

void SubaddressAccount::clearRows()
{
    qDeleteAll(m_rows);
    m_rows.clear();
}

AccountRow* SubaddressAccount::row(int index) const
{
    return m_rows.value(index);
}