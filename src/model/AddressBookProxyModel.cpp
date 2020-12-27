// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "AddressBookProxyModel.h"
#include "AddressBookModel.h"

AddressBookProxyModel::AddressBookProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_searchRegExp("")
{
    m_searchRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    m_searchRegExp.setPatternSyntax(QRegExp::RegExp);
}

bool AddressBookProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex addressIndex = sourceModel()->index(sourceRow, AddressBookModel::Address, sourceParent);
    QModelIndex descriptionIndex = sourceModel()->index(sourceRow, AddressBookModel::Description, sourceParent);

    QString addressData = sourceModel()->data(addressIndex, Qt::UserRole).toString();
    QString descriptionData = sourceModel()->data(descriptionIndex).toString();

    return (addressData.contains(m_searchRegExp) || descriptionData.contains(m_searchRegExp));
}