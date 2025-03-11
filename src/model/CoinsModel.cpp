// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "CoinsModel.h"
#include "libwalletqt/rows/CoinsInfo.h"
#include "Coins.h"
#include "constants.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/Utils.h"
#include "libwalletqt/WalletManager.h"

#include <QBrush>

CoinsModel::CoinsModel(QObject *parent, Coins *coins)
        : QAbstractTableModel(parent)
        , m_coins(coins)
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
    const QList<CoinsInfo>& rows = m_coins->getRows();
    if (index.row() < 0 || index.row() >= rows.size()) {
        return {};
    }
    const CoinsInfo& row = rows[index.row()];

    bool selected = row.keyImageKnown && m_selected.contains(row.keyImage);

    if(role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        return parseTransactionInfo(row, index.column(), role);
    }
    else if (role == Qt::BackgroundRole) {
        if (row.spent) {
            return QBrush(ColorScheme::RED.asColor(true));
        }
        if (row.frozen) {
            return QBrush(ColorScheme::BLUE.asColor(true));
        }
        if (!row.unlocked) {
            return QBrush(ColorScheme::YELLOW.asColor(true));
        }
        if (selected) {
            return QBrush(ColorScheme::GREEN.asColor(true));
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case Amount:
                return Qt::AlignRight;
        }
    }
    else if (role == Qt::DecorationRole) {
        switch (index.column()) {
            case KeyImageKnown:
            {
                if (row.keyImageKnown) {
                    return QVariant(icons()->icon("eye1.png"));
                }
                return QVariant(icons()->icon("eye_blind.png"));
            }
        }
    }
    else if (role == Qt::FontRole) {
        switch(index.column()) {
            case PubKey:
            case TxID:
            case Address:
                return Utils::getMonospaceFont();
        }
    }
    else if (role == Qt::ToolTipRole) {
        switch(index.column()) {
            case KeyImageKnown:
            {
                if (row.keyImageKnown) {
                    return "Key image known";
                }
                return "Key image unknown. Outgoing transactions that include this output will not be detected.";
            }
        }
        if (row.frozen) {
            return "Output is frozen.";
        }
        if (!row.unlocked) {
            return "Output is locked (needs more confirmations)";
        }
        if (row.spent) {
            return "Output is spent";
        }
        if (selected) {
            return "Coin selected to be spent";
        }
    }

    return {};
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
            case TxID:
                return QString("TxID");
            case BlockHeight:
                return QString("Height");
            case Address:
                return QString("Address");
            case Label:
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

Qt::ItemFlags CoinsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Label)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}

bool CoinsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const CoinsInfo& row = m_coins->getRow(index.row());

        QString pubkey = row.pubKey;

        switch (index.column()) {
            case Label:
                m_coins->setDescription(pubkey, m_currentSubaddressAccount, value.toString());
                emit descriptionChanged();
                break;
            default:
                return false;
        }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }

    return false;
}

QVariant CoinsModel::parseTransactionInfo(const CoinsInfo &cInfo, int column, int role) const
{
    switch (column)
    {
        case KeyImageKnown:
            return "";
        case PubKey:
            return cInfo.pubKey.mid(0,8);
        case TxID:
            return cInfo.hash.mid(0, 8) + " ";
        case BlockHeight:
            return cInfo.blockHeight;
        case Address:
            return Utils::displayAddress(cInfo.address, 1, "");
        case Label: {
            if (!cInfo.description.isEmpty())
                return cInfo.description;
            if (!cInfo.txNote.isEmpty())
                return cInfo.txNote;
            return cInfo.getAddressLabel();
        }
        case Spent:
            return cInfo.spent;
        case SpentHeight:
            return cInfo.spentHeight;
        case Amount:
        {
            if (role == Qt::UserRole) {
                return cInfo.amount;
            }
            return cInfo.displayAmount();
        }
        case Frozen:
            return cInfo.frozen;
        default:
        {
            qCritical() << "Unimplemented role";
            return QVariant();
        }
    }
}

void CoinsModel::setCurrentSubaddressAccount(quint32 accountIndex) {
    m_currentSubaddressAccount = accountIndex;
}

void CoinsModel::setSelected(const QStringList &keyimages) {
    m_selected.clear();
    for (const auto &ki : keyimages) {
        m_selected.insert(ki);
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

const CoinsInfo& CoinsModel::entryFromIndex(const QModelIndex &index) const {
    Q_ASSERT(index.isValid() && index.row() < m_coins->count());
    return m_coins->getRow(index.row());
}
