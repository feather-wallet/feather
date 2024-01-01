// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "BountiesProxyModel.h"

BountiesProxyModel::BountiesProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::UserRole);
}
