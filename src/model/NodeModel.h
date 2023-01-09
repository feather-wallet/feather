// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_NODEMODEL_H
#define FEATHER_NODEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

class FeatherNode;

class NodeModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum ModelColumn {
        URL,
        Height,
        COUNT
    };

    explicit NodeModel(int nodeSource, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    FeatherNode node(int row);

    void clear();
    void updateNodes(QList<FeatherNode> nodes);

private:
    QList<FeatherNode> m_nodes;
    int m_nodeSource;
    QIcon m_offline;
    QIcon m_online;
};

#endif //FEATHER_NODEMODEL_H
