// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TransactionHistoryProxyModel.h"
#include "TransactionHistoryModel.h"

#include "libwalletqt/rows/TransactionRow.h"

TransactionHistoryProxyModel::TransactionHistoryProxyModel(Wallet *wallet, QObject *parent)
        : QSortFilterProxyModel(parent)
        , m_wallet(wallet)
        , m_searchRegExp("")
{
    m_searchRegExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    m_history = m_wallet->history();
}

TransactionHistory* TransactionHistoryProxyModel::history() {
    return m_history;
}

bool TransactionHistoryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceRow < 0 || sourceRow >= m_history->count()) {
        return false;
    }

    const TransactionRow& row = m_history->transaction(sourceRow);

    QString description = row.description;
    QString txid = row.hash;
    QString subaddrlabel = row.label;
    quint32 subaddrAccount = row.subaddrAccount;
    QSet<quint32> subaddrIndex = row.subaddrIndex;

    bool addressFound;
    for (quint32 i : subaddrIndex) {
        QString address = m_wallet->address(subaddrAccount, i);
        addressFound = address.contains(m_searchRegExp);
        if (addressFound) break;
    }
    
    return (description.contains(m_searchRegExp) || txid.contains(m_searchRegExp) || subaddrlabel.contains(m_searchRegExp)) || addressFound;
}
