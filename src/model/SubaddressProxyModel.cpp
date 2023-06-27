// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SubaddressProxyModel.h"

SubaddressProxyModel::SubaddressProxyModel(QObject *parent, Subaddress *subaddress, bool showChange)
    : QSortFilterProxyModel(parent)
    , m_subaddress(subaddress)
    , m_searchRegExp("")
    , m_searchCaseSensitiveRegExp("")
    , m_showChange(showChange)
{
    m_searchRegExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
}

bool SubaddressProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString address, label;
    bool isUsed;
    m_subaddress->getRow(sourceRow, [&isUsed, &address, &label](const Monero::SubaddressRow &subaddress){
        isUsed = subaddress.isUsed();
        address = QString::fromStdString(subaddress.getAddress());
        label = QString::fromStdString(subaddress.getLabel());
    });

    // Hide primary/change addresses
    if (!m_showChange && sourceRow == 0) {
        return false;
    }

    if (!m_showHidden && m_hiddenAddresses.contains(address)) {
        return false;
    }

    if (!m_searchRegExp.pattern().isEmpty()) {
        return address.contains(m_searchCaseSensitiveRegExp) || label.contains(m_searchRegExp);
    }
    return (m_showUsed || !isUsed);
}
