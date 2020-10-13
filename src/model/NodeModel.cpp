// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "NodeModel.h"
#include <utils/nodes.h>
#include "appcontext.h"

NodeModel::NodeModel(unsigned int nodeSource, QObject *parent)
        : QAbstractTableModel(parent)
        , m_nodeSource(nodeSource)
        , m_offline(QIcon(":/assets/images/expired_icon.png"))
        , m_online(QIcon(":/assets/images/confirmed_icon.png"))
{
}

void NodeModel::clear() {
    beginResetModel();
    m_nodes.clear();
    endResetModel();
}

void NodeModel::updateNodes(const QList<FeatherNode> nodes) {
    beginResetModel();
    m_nodes.clear();
    m_nodes = nodes;
    endResetModel();
}

int NodeModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;
    return m_nodes.count();
}

int NodeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_nodeSource == NodeSource::websocket ? 2 : 1;
}

QVariant NodeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_nodes.count())
        return QVariant();

    FeatherNode node = m_nodes.at(index.row());

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case NodeModel::URL:
                return node.full;
            case NodeModel::Height:
                if(node.online)
                    return node.height == 0 ? QString("-") : QString::number(node.height);
                return QString("-");
            default:
                return QVariant();
        }
    }
    else if (role == Qt::DecorationRole) {
        switch (index.column()) {
            case NodeModel::URL: {
                if(m_nodeSource == NodeSource::websocket)
                    return QVariant(node.online ? m_online : m_offline);
                return QVariant();
            }
            default: {
                return QVariant();
            }
        }
    }
    else if(role == Qt::BackgroundRole) {
        if (node.isConnecting)
            return QBrush(QColor(186, 247, 255));
        else if (node.isActive)
            return QBrush(QColor(158, 250, 158));
    }
    return QVariant();
}

QVariant NodeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch(section) {
            case NodeModel::URL:
                return QString("Node");
            case NodeModel::Height:
                return QString("Height");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

FeatherNode NodeModel::node(int row) {
    if (row < 0 || row >= m_nodes.size()) {
        qCritical("%s: no reddit post for index %d", __FUNCTION__, row);
        return FeatherNode("", 0, false);
    }
    return m_nodes.at(row);
}
