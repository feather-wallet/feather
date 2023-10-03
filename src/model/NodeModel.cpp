// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "NodeModel.h"
#include "utils/nodes.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"

NodeModel::NodeModel(int nodeSource, QObject *parent)
        : QAbstractTableModel(parent)
        , m_nodeSource(nodeSource)
        , m_offline(icons()->icon("status_offline.svg"))
        , m_online(icons()->icon("status_connected.svg"))
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
                return node.toFullAddress();
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
                if (m_nodeSource == NodeSource::websocket && !conf()->get(Config::offlineMode).toBool()) {
                    return QVariant(node.online ? m_online : m_offline);
                }
                return QVariant();
            }
            default: {
                return QVariant();
            }
        }
    }
    else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case NodeModel::URL: {
                if (node.isConnecting)
                    return QString("Feather is connecting to this node.");
                if (node.isActive)
                    return QString("Feather is connected to this node.");
            }
        }
    }
    else if (role == Qt::BackgroundRole) {
        if (node.isConnecting)
            return QBrush(ColorScheme::YELLOW.asColor(true));
        else if (node.isActive)
            return QBrush(ColorScheme::GREEN.asColor(true));
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
                return QString("Height ");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

FeatherNode NodeModel::node(int row) {
    if (row < 0 || row >= m_nodes.size()) {
        qCritical("%s: no reddit post for index %d", __FUNCTION__, row);
        return FeatherNode();
    }
    return m_nodes.at(row);
}
