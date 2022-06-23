// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_BOUNTIESMODEL_H
#define FEATHER_BOUNTIESMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "widgets/Bounty.h"

class BountiesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
        Title = 0,
        Votes,
        Status,
        Bounty,
        COUNT
    };

    explicit BountiesModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updateBounties(const QList<QSharedPointer<BountyEntry>>& posts);

    QSharedPointer<BountyEntry> post(int row);

private:
    QList<QSharedPointer<BountyEntry>> m_bounties;
};


#endif //FEATHER_BOUNTIESMODEL_H
