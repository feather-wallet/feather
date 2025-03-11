// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

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

    if (sourceRow < 0 || sourceRow >= m_subaddress->count()) {
        return false;
    }

    const SubaddressRow& subaddress = m_subaddress->row(sourceRow);
    
    // Pinned addresses are always shown
    if (subaddress.pinned) {
        return true;
    }
    
    // Hide primary/change addresses
    if (!showChange && sourceRow == 0) {
        return false;
    }

    if (!showHidden && subaddress.hidden) {
        return false;
    }

    if (!m_searchRegExp.pattern().isEmpty()) {
        return subaddress.address.contains(m_searchCaseSensitiveRegExp) || subaddress.label.contains(m_searchRegExp);
    }

    return (showUsed || !subaddress.used);
}
