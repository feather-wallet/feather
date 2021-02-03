// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CoinsProxyModel.h"
#include "CoinsModel.h"
#include "libwalletqt/CoinsInfo.h"

CoinsProxyModel::CoinsProxyModel(QObject *parent, Coins *coins)
    : QSortFilterProxyModel(parent), m_coins(coins)
{
    setSortRole(Qt::UserRole);
}

bool CoinsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool isSpent;
    int accountIndex;
    m_coins->coin(sourceRow, [&isSpent, &accountIndex](const CoinsInfo &c){
        isSpent = c.spent();
        accountIndex = c.subaddrAccount();
    });

    return !(!m_showSpent && isSpent) && accountIndex == 0;
}