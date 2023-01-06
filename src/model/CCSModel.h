// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_CCSMODEL_H
#define FEATHER_CCSMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "widgets/CCSEntry.h"

class CCSModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Title = 0,
        Author,
        Progress,
        COUNT
    };

    explicit CCSModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updateEntries(const QList<QSharedPointer<CCSEntry>>& entries);

    QSharedPointer<CCSEntry> entry(int row);

private:
    QList<QSharedPointer<CCSEntry>> m_entries;
};


#endif //FEATHER_CCSMODEL_H
