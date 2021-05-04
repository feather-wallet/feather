// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CoinsModel.h"
#include "CoinsInfo.h"
#include "Coins.h"
#include "ModelUtils.h"
#include "globals.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"

#include <QBrush>

CoinsModel::CoinsModel(QObject *parent, Coins *coins)
        : QAbstractTableModel(parent),
          m_coins(coins)
{
    connect(m_coins, &Coins::refreshStarted, this, &CoinsModel::startReset);
    connect(m_coins, &Coins::refreshFinished, this, &CoinsModel::endReset);
}

void CoinsModel::startReset(){
    beginResetModel();
}

void CoinsModel::endReset(){
    endResetModel();
}

Coins * CoinsModel::coins() const {
    return m_coins;
}

int CoinsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_coins->count();
    }
}

int CoinsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant CoinsModel::data(const QModelIndex &index, int role) const
{
    if (!m_coins) {
        return QVariant();
    }

    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_coins->count())
        return QVariant();

    QVariant result;

    bool found = m_coins->coin(index.row(), [this, &index, &result, &role](const CoinsInfo &cInfo) {
        if(role == Qt::DisplayRole || role == Qt::UserRole) {
            result = parseTransactionInfo(cInfo, index.column(), role);
        }
        else if (role == Qt::BackgroundRole) {
            if (cInfo.spent()) {
                result = QBrush(ColorScheme::RED.asColor(true));
            }
            else if (cInfo.frozen()) {
                result = QBrush(ColorScheme::BLUE.asColor(true));
            }
            else if (!cInfo.unlocked()) {
                result = QBrush(ColorScheme::YELLOW.asColor(true));
            }
        }
        else if (role == Qt::TextAlignmentRole) {
            switch (index.column()) {
                case Amount:
                    result = Qt::AlignRight;
            }
        }
        else if (role == Qt::DecorationRole) {
            switch (index.column()) {
                case KeyImageKnown:
                {
                    if (cInfo.keyImageKnown()) {
                        result = QVariant(icons()->icon("eye1.png"));
                    }
                    else {
                        result = QVariant(icons()->icon("eye_blind.png"));
                    }
                }
            }
        }
        else if (role == Qt::FontRole) {
            switch(index.column()) {
                case PubKey:
                case OutputPoint:
                case Address:
                    result = ModelUtils::getMonospaceFont();
            }
        }
        else if (role == Qt::ToolTipRole) {
            switch(index.column()) {
                case KeyImageKnown:
                {
                    if (cInfo.keyImageKnown()) {
                        result = "Key image known";
                    } else {
                        result = "Key image unknown. Outgoing transactions that include this output will not be detected.";
                    }
                }
            }
            if (cInfo.frozen()) {
                result = "Output is frozen.";
            }
            else if (!cInfo.unlocked()) {
                result = "Output is locked (needs more confirmations)";
            }
        }
    });
    if (!found) {
        qCritical("%s: internal error: no transaction info for index %d", __FUNCTION__, index.row());
    }
    return result;
}

QVariant CoinsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case PubKey:
                return QString("Pub Key");
            case OutputPoint:
                return QString("Output point");
            case BlockHeight:
                return QString("Height");
            case Address:
                return QString("Address");
            case AddressLabel:
                return QString("Label");
            case SpentHeight:
                return QString("Spent Height");
            case Amount:
                return QString("Amount");
            case Spent:
                return QString("Spent");
            case Frozen:
                return QString("Frozen");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant CoinsModel::parseTransactionInfo(const CoinsInfo &cInfo, int column, int role) const
{
    switch (column)
    {
        case KeyImageKnown:
            return "";
        case PubKey:
            return cInfo.pubKey().mid(0,8);
        case OutputPoint:
            return cInfo.hash().mid(0, 8) + ":" + QString::number(cInfo.internalOutputIndex());
        case BlockHeight:
            return cInfo.blockHeight();
        case Address:
            return ModelUtils::displayAddress(cInfo.address(), 1, "");
        case AddressLabel:
            return cInfo.addressLabel();
        case Spent:
            return cInfo.spent();
        case SpentHeight:
            return cInfo.spentHeight();
        case Amount:
        {
            if (role == Qt::UserRole) {
                return cInfo.amount() / globals::cdiv;
            }
            return QString::number(cInfo.amount() / globals::cdiv, 'f', 12);
        }
        case Frozen:
            return cInfo.frozen();
        default:
        {
            qCritical() << "Unimplemented role";
            return QVariant();
        }
    }
}

CoinsInfo* CoinsModel::entryFromIndex(const QModelIndex &index) const {
    Q_ASSERT(index.isValid() && index.row() < m_coins->count());
    return m_coins->coin(index.row());
}