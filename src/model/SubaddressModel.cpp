// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

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
    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_subaddress->count())
        return QVariant();

    QVariant result;
    
    bool found = m_subaddress->getRow(index.row(), [this, &index, &role, &result](const SubaddressRow &subaddress) {
        if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole){
            result = parseSubaddressRow(subaddress, index, role);
        }
        
        else if (role == Qt::DecorationRole) {
            if (subaddress.isPinned() && index.column() == ModelColumn::Index) {
                result = QVariant(icons()->icon("pin.png"));
            }
            else if (subaddress.isHidden() && index.column() == ModelColumn::Index) {
                result = QVariant(icons()->icon("eye_blind.png"));
            }
        }
        else if (role == Qt::BackgroundRole) {
            switch(index.column()) {
                case Address:
                {
                    if (subaddress.isUsed()) {
                        result = QBrush(ColorScheme::RED.asColor(true));
                    }
                }
            }
        }
        else if (role == Qt::FontRole) {
            switch(index.column()) {
                case Address:
                {
                   result = Utils::getMonospaceFont();
                }
            }
        }
        else if (role == Qt::ToolTipRole) {
            switch(index.column()) {
                case Address:
                {
                    if (subaddress.isUsed()) {
                        result = "This address is used.";
                    }
                }
            }
        }
    });

    if (!found)
    {
        qCritical("%s: internal error: invalid index %d", __FUNCTION__, index.row());
    }

    return result;
}

QVariant SubaddressModel::parseSubaddressRow(const SubaddressRow &subaddress, const QModelIndex &index, int role) const
{
    bool showFull = conf()->get(Config::showFullAddresses).toBool();
    switch (index.column()) {
        case Index:
        {
            if (role == Qt::UserRole) {
                if (subaddress.isPinned()) {
                    return -1;
                } else {
                    return subaddress.getRow();
                }
            }
            return "#" + QString::number(subaddress.getRow()) + " ";
        }
        case Address:
        {
            QString address = subaddress.getAddress();
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
            return subaddress.getLabel();
        }
        case isUsed:
            return subaddress.isUsed();
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

SubaddressRow* SubaddressModel::entryFromIndex(const QModelIndex &index) const {
    Q_ASSERT(index.isValid() && index.row() < m_subaddress->count());
    return m_subaddress->row(index.row());
}