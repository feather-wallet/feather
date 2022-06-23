// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "BountiesModel.h"

BountiesModel::BountiesModel(QObject *parent)
        : QAbstractTableModel(parent)
{

}

void BountiesModel::clear() {
    beginResetModel();

    m_bounties.clear();

    endResetModel();
}

void BountiesModel::updateBounties(const QList<QSharedPointer<BountyEntry>> &posts) {
    beginResetModel();

    m_bounties.clear();
    for (const auto& post : posts) {
        m_bounties.push_back(post);
    }

    endResetModel();
}

int BountiesModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid()) {
        return 0;
    }
    return m_bounties.count();
}

int BountiesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant BountiesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_bounties.count())
        return {};

    QSharedPointer<BountyEntry> post = m_bounties.at(index.row());

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case Votes:
                return QString::number(post->votes);
            case Title:
                return post->title;
            case Status:
                return post->status;
            case Bounty: {
                if (post->bountyAmount > 0) {
                    return QString("%1 XMR").arg(QString::number(post->bountyAmount, 'f', 5));
                }
                return "None";
            }
            default:
                return {};
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch(index.column()) {
            case Votes:
            case Status:
            case Bounty:
                return Qt::AlignRight;
            default:
                return {};
        }
    }
    return {};
}

QVariant BountiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Votes:
                return QString("🡅");
            case Title:
                return QString("Title");
            case Status:
                return QString("Status");
            case Bounty:
                return QString("Bounty");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QSharedPointer<BountyEntry> BountiesModel::post(int row) {
    if (row < 0 || row >= m_bounties.size()) {
        qCritical("%s: no reddit post for index %d", __FUNCTION__, row);
        return QSharedPointer<BountyEntry>();
    }

    return m_bounties.at(row);
}