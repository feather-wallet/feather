// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

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
    QString description, txid, subaddrlabel;
    quint32 subaddrAccount;
    QSet<quint32> subaddrIndex;

    m_history->transaction(sourceRow, [&description, &txid, &subaddrlabel, &subaddrAccount, &subaddrIndex](TransactionRow &tInfo){
        description = tInfo.description();
        txid = tInfo.hash();
        subaddrlabel = tInfo.label();
        subaddrAccount = tInfo.subaddrAccount();
        subaddrIndex = tInfo.subaddrIndex();
    });

    bool addressFound;
    for (quint32 i : subaddrIndex) {
        QString address = m_wallet->address(subaddrAccount, i);
        addressFound = address.contains(m_searchRegExp);
        if (addressFound) break;
    }
    
    return (description.contains(m_searchRegExp) || txid.contains(m_searchRegExp) || subaddrlabel.contains(m_searchRegExp)) || addressFound;
}