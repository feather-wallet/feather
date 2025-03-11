// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SubaddressModel.h"
#include "Subaddress.h"

#include <QPoint>
#include <QColor>
#include <QBrush>

#include "utils/config.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

SubaddressModel::SubaddressModel(QObject *parent, Subaddress *subaddress)
    : QAbstractTableModel(parent)
    , m_subaddress(subaddress)
{
    connect(m_subaddress, &Subaddress::refreshStarted, this, &SubaddressModel::beginResetModel);
    connect(m_subaddress, &Subaddress::refreshFinished, this, &SubaddressModel::endResetModel);
}

int SubaddressModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_subaddress->count();
    }
}

int SubaddressModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant SubaddressModel::data(const QModelIndex &index, int role) const
{
    const QList<SubaddressRow>& rows = m_subaddress->getRows();
    if (index.row() < 0 || index.row() >= rows.size()) {
        return {};
    }
    const SubaddressRow& row = rows[index.row()];
    
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole){
        return parseSubaddressRow(row, index, role);
    }

    else if (role == Qt::DecorationRole) {
        if (row.pinned && index.column() == ModelColumn::Index) {
            return QVariant(icons()->icon("pin.png"));
        }
        else if (row.hidden && index.column() == ModelColumn::Index) {
            return QVariant(icons()->icon("eye_blind.png"));
        }
    }
    else if (role == Qt::BackgroundRole) {
        switch(index.column()) {
            case Address:
            {
                if (row.used) {
                    return QBrush(ColorScheme::RED.asColor(true));
                }
            }
        }
    }
    else if (role == Qt::FontRole) {
        switch(index.column()) {
            case Address:
            {
               return Utils::getMonospaceFont();
            }
        }
    }
    else if (role == Qt::ToolTipRole) {
        switch(index.column()) {
            case Address:
            {
                if (row.used) {
                    return "This address is used.";
                }
            }
        }
    }

    return {};
}

QVariant SubaddressModel::parseSubaddressRow(const SubaddressRow &subaddress, const QModelIndex &index, int role) const
{
    bool showFull = conf()->get(Config::showFullAddresses).toBool();
    switch (index.column()) {
        case Index:
        {
            if (role == Qt::UserRole) {
                if (subaddress.pinned) {
                    return 0;
                } else {
                    return index.row() + 1;
                }
            }
            return "#" + QString::number(index.row()) + " ";
        }
        case Address:
        {
            QString address = subaddress.address;
            if (!showFull && role != Qt::UserRole) {
                address = Utils::displayAddress(address);
            }
            return address;
        }
        case Label:
        {
            if (m_currentSubaddressAccount == 0 && index.row() == 0) {
                return "Primary address";
            }
            else if (index.row() == 0) {
                return "Change";
            }
            return subaddress.label;
        }
        case isUsed:
            return subaddress.used;
        default:
            qCritical() << "Invalid column" << index.column();
            return QVariant();
    }
}


QVariant SubaddressModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Index:
                return QString("#");
            case Address:
                return QString("Address");
            case Label:
                return QString("Label");
            case isUsed:
                return QString("Used");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool SubaddressModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();

        switch (index.column()) {
            case Label:
                m_subaddress->setLabel(m_currentSubaddressAccount, row, value.toString());
                break;
            default:
                return false;
        }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }
    return false;
}

Qt::ItemFlags SubaddressModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Label && index.row() != 0)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}

void SubaddressModel::setCurrentSubaddressAccount(quint32 accountIndex) {
    m_currentSubaddressAccount = accountIndex;
}

const SubaddressRow& SubaddressModel::entryFromIndex(const QModelIndex &index) const {
    Q_ASSERT(index.isValid() && index.row() < m_subaddress->count());
    return m_subaddress->row(index.row());
}
