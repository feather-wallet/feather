// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "TransactionHistoryProxyModel.h"
#include "TransactionHistoryModel.h"

#include "libwalletqt/TransactionInfo.h"

TransactionHistoryProxyModel::TransactionHistoryProxyModel(Wallet *wallet, QObject *parent)
        : QSortFilterProxyModel(parent),
        m_wallet(wallet),
        m_searchRegExp("")
{
    m_searchRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    m_searchRegExp.setPatternSyntax(QRegExp::RegExp);
}

bool TransactionHistoryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex descriptionIndex = sourceModel()->index(sourceRow, TransactionHistoryModel::Description, sourceParent);
    QModelIndex txidIndex = sourceModel()->index(sourceRow, TransactionHistoryModel::TxID, sourceParent);

    QString descriptionData = sourceModel()->data(descriptionIndex).toString();
    QString txidData = sourceModel()->data(txidIndex).toString();

    quint32 subaddrAcount;
    QSet<quint32> subaddrIndex;

    m_wallet->history()->transaction(sourceRow, [&subaddrAcount, &subaddrIndex](TransactionInfo &tInfo){
        subaddrAcount = tInfo.subaddrAccount();
        subaddrIndex = tInfo.subaddrIndex();
    });

    bool addressFound;
    for (quint32 i : subaddrIndex) {
        QString address = m_wallet->address(subaddrAcount, i);
        addressFound = address.contains(m_searchRegExp);
        if (addressFound) break;
    }
    
    return (descriptionData.contains(m_searchRegExp) || txidData.contains(m_searchRegExp)) || addressFound;
}