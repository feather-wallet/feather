// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "SubaddressProxyModel.h"

SubaddressProxyModel::SubaddressProxyModel(QObject *parent, Subaddress *subaddress, bool hidePrimary)
    : QSortFilterProxyModel(parent),
    m_searchRegExp(""),
    m_searchCaseSensitiveRegExp(""),
    m_subaddress(subaddress),
    m_hidePrimary(hidePrimary)
{
    m_searchRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    m_searchRegExp.setPatternSyntax(QRegExp::FixedString);
    m_searchCaseSensitiveRegExp.setPatternSyntax(QRegExp::FixedString);
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

    // Hide primary address
    if (sourceRow == 0 && m_hidePrimary)
        return false;

    if (!m_searchRegExp.isEmpty()) {
        return address.contains(m_searchCaseSensitiveRegExp) || label.contains(m_searchRegExp);
    }
    return (m_showUsed || !isUsed);
}
