// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef SUBADDRESSMODEL_H
#define SUBADDRESSMODEL_H

#include <QAbstractTableModel>

#include "rows/SubaddressRow.h"

class Subaddress;

class SubaddressModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
         Index = 0,
         Address,
         Label,
         isUsed,
         COUNT
    };

    explicit SubaddressModel(QObject *parent, Subaddress *subaddress);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    const SubaddressRow& entryFromIndex(const QModelIndex &index) const;

    void rowUpdated(qsizetype index);
    void beginRowAdded(qsizetype index);

private:
    Subaddress *m_subaddress;
    QVariant parseSubaddressRow(const SubaddressRow &subaddress, const QModelIndex &index, int role) const;

    quint32 m_currentSubaddressAccount;
};

#endif // SUBADDRESSMODEL_H
