// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef BOUNTIESPROXYMODEL_H
#define BOUNTIESPROXYMODEL_H

#include <QSortFilterProxyModel>

class BountiesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit BountiesProxyModel(QObject* parent = nullptr);
};

#endif //BOUNTIESPROXYMODEL_H
