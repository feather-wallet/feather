// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SubaddressProxyModel.h"

#include "utils/config.h"

SubaddressProxyModel::SubaddressProxyModel(QObject *parent, Subaddress *subaddress)
    : QSortFilterProxyModel(parent)
    , m_subaddress(subaddress)
    , m_searchRegExp("")
    , m_searchCaseSensitiveRegExp("")
{
    m_searchRegExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    this->setSortRole(Qt::UserRole);
    this->sort(0);
}

bool SubaddressProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool showUsed = conf()->get(Config::showUsedAddresses).toBool();
    bool showHidden = conf()->get(Config::showHiddenAddresses).toBool();
    bool showChange = conf()->get(Config::showChangeAddresses).toBool();

    SubaddressRow* subaddress = m_subaddress->row(sourceRow);
    if (!subaddress) {
        return false;
    }
    
    // Pinned addresses are always shown
    if (subaddress->isPinned()) {
        return true;
    }
    
    // Hide primary/change addresses
    if (!showChange && sourceRow == 0) {
        return false;
    }

    if (!showHidden && subaddress->isHidden()) {
        return false;
    }

    if (!m_searchRegExp.pattern().isEmpty()) {
        return subaddress->getAddress().contains(m_searchCaseSensitiveRegExp) || subaddress->getLabel().contains(m_searchRegExp);
    }

    return (showUsed || !subaddress->isUsed());
}
