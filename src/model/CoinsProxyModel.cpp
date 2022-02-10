// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "CoinsProxyModel.h"
#include "CoinsModel.h"
#include "libwalletqt/CoinsInfo.h"

CoinsProxyModel::CoinsProxyModel(QObject *parent, Coins *coins)
        : QSortFilterProxyModel(parent)
        , m_coins(coins)
        , m_searchRegExp("")
{
    m_searchRegExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    setSortRole(Qt::UserRole);
}

void CoinsProxyModel::setShowSpent(const bool showSpent) {
    m_showSpent = showSpent;
    invalidateFilter();
}

void CoinsProxyModel::setSearchFilter(const QString &searchString) {
    m_searchRegExp.setPattern(searchString);
    invalidateFilter();
}

bool CoinsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    CoinsInfo* coin = m_coins->coin(sourceRow);

    if (!m_showSpent && coin->spent()) {
        return false;
    }

    if (!m_searchRegExp.pattern().isEmpty()) {
        return coin->pubKey().contains(m_searchRegExp) || coin->address().contains(m_searchRegExp)
                || coin->hash().contains(m_searchRegExp) || coin->addressLabel().contains(m_searchRegExp)
                || coin->description().contains(m_searchRegExp);
    }

    return true;
}