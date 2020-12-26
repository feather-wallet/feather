// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CoinsProxyModel.h"
#include "CoinsModel.h"

CoinsProxyModel::CoinsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::UserRole);
}

bool CoinsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex spentIndex = sourceModel()->index(sourceRow, CoinsModel::Spent, sourceParent);
    bool isSpent = sourceModel()->data(spentIndex).toBool();

    return !(!m_showSpent && isSpent);
}