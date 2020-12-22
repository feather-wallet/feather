// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "CCSModel.h"

CCSModel::CCSModel(QObject *parent)
        : QAbstractTableModel(parent)
{

}

void CCSModel::clear() {
    beginResetModel();

    m_entries.clear();

    endResetModel();
}

void CCSModel::updateEntries(const QList<QSharedPointer<CCSEntry>>& entries) {
    beginResetModel();

    m_entries.clear();
    for (const auto& entry : entries) {
        m_entries.push_back(entry);
    }

    endResetModel();
}

int CCSModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.count();
}

int CCSModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant CCSModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.count())
        return QVariant();

    QSharedPointer<CCSEntry> entry = m_entries.at(index.row());

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case Title:
                return entry->title;
            case Author:
                return entry->author;
#ifdef Q_OS_MACOS
            case Progress:
                return QString("%1/%2 XMR").arg(entry->raised_amount).arg(entry->target_amount);
#endif
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant CCSModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Title:
                return QString("Community Crowdfunding Proposal");
            case Author:
                return QString("Author");
            case Progress:
                return QString("Progress");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QSharedPointer<CCSEntry> CCSModel::entry(int row) {
    if (row < 0 || row >= m_entries.size()) {
        qCritical("%s: no reddit post for index %d", __FUNCTION__, row);
        return QSharedPointer<CCSEntry>();
    }

    return m_entries.at(row);
}