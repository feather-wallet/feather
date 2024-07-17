// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "libwalletqt/rows/MultisigMessage.h"
#include "MultisigMessageModel.h"
#include "MultisigMessageStore.h"
#include "constants.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/Utils.h"
#include "libwalletqt/WalletManager.h"

#include <QBrush>

MultisigMessageModel::MultisigMessageModel(QObject *parent, MultisigMessageStore *store)
        : QAbstractTableModel(parent)
        , m_store(store)
{
    connect(m_store, &MultisigMessageStore::refreshStarted, this, &MultisigMessageModel::startReset);
    connect(m_store, &MultisigMessageStore::refreshFinished, this, &MultisigMessageModel::endReset);
}

void MultisigMessageModel::startReset(){
    beginResetModel();
}

void MultisigMessageModel::endReset(){
    endResetModel();
}

int MultisigMessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_store->count();
    }
}

int MultisigMessageModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant MultisigMessageModel::data(const QModelIndex &index, int role) const
{
    if (!m_store) {
        return QVariant();
    }

    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_store->count())
        return QVariant();

    QVariant result;

    bool found = m_store->message(index.row(), [this, &index, &result, &role](const MultisigMessage &msg) {
        if(role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
            result = getMessageInfo(msg, index.column(), role);
        }
    });
    if (!found) {
        qCritical("%s: internal error: no transaction info for index %d", __FUNCTION__, index.row());
    }
    return result;
}

QVariant MultisigMessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Id:
                return QString("Id");
            case Type:
                return QString("Type");
            case Direction:
                return QString("Direction");
            case Created:
                return QString("Created");
            case Modified:
                return QString("Modified");
            case Sent:
                return QString("Sent");
            case Signer:
                return QString("Signer");
            case Hash:
                return QString("Hash");
            case State:
                return QString("State");
            case WalletHeight:
                return QString("Wallet Height");
            case Round:
                return QString("Round");
            case SignatureCount:
                return QString("Signature Count");
            case TransportId:
                return QString("Transport ID");
            default:
                return QVariant();
        }
    }
    return QVariant();
}


QVariant MultisigMessageModel::getMessageInfo(const MultisigMessage &msg, int column, int role) const
{
    switch (column)
    {
        case Id:
            return msg.id;
        case Type:
            return msg.type;
        case Direction:
            return msg.direction;
        case Created:
            return msg.created;
        case Modified:
            return msg.modified;
        case Sent:
            return msg.sent;
        case Signer:
            return msg.signer;
        case Hash: {
            return msg.hash;
        }
        case State:
            return msg.state;
        case WalletHeight:
            return msg.wallet_height;
        case Round:
        {
            return msg.round;
        }
        case SignatureCount:
            return msg.signature_count;
        case TransportId:
            return msg.transport_id;
        default:
        {
            qCritical() << "Unimplemented role";
            return QVariant();
        }
    }
}

