////
//// Created by user on 1/9/24.
////
//

#include "MultisigIncomingTxModel.h"
#include "libwalletqt/rows/MultisigMessage.h"
#include "MultisigMessageStore.h"
#include "constants.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/Utils.h"
#include "libwalletqt/WalletManager.h"
#include "rows/TxProposal.h"
#include "config.h"

#include <QBrush>

MultisigIncomingTxModel::MultisigIncomingTxModel(QObject *parent, MultisigMessageStore *store)
        : QAbstractTableModel(parent)
        , m_store(store)
{
    connect(m_store, &MultisigMessageStore::refreshStarted, this, &MultisigIncomingTxModel::startReset);
    connect(m_store, &MultisigMessageStore::refreshFinished, this, &MultisigIncomingTxModel::endReset);
}

void MultisigIncomingTxModel::startReset(){
    beginResetModel();
}

void MultisigIncomingTxModel::endReset(){
    endResetModel();
}

int MultisigIncomingTxModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_store->txProposalCount();
    }
}

int MultisigIncomingTxModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant MultisigIncomingTxModel::data(const QModelIndex &index, int role) const
{
    if (!m_store) {
        return QVariant();
    }

    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_store->count())
        return QVariant();

    QVariant result;

    bool found = m_store->txProposal(index.row(), [this, &index, &result, &role](const TxProposal &msg) {
        if(role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
            result = getMessageInfo(msg, index.column(), role);
        }
        else if (role == Qt::DecorationRole) {
            switch (index.column()) {
                case ModelColumn::Status:
                {
                    switch (msg.status) {
                        case TxProposal::Status::Completed: {
                            result = QVariant(icons()->icon("confirmed.svg"));
                            break;
                        }
                        case TxProposal::Status::Cant_Sign:
                        case TxProposal::Status::Double_Spend:
                        {
                            result = QVariant(icons()->icon("expired.png"));
                            break;
                        }
                        case TxProposal::Status::Frozen: {
                            result = QVariant(icons()->icon("freeze.png"));
                            break;
                        }
                        case TxProposal::Status::Signed: {
                            result = QVariant(icons()->icon("sign.png"));
                            break;
                        }
                        default: {
                            result = QVariant(icons()->icon("arrow.svg"));
                        }
                    }
                }
            }
        }

    });
    if (!found) {
        qCritical("%s: internal error: no transaction info for index %d", __FUNCTION__, index.row());
    }
    return result;
}

QVariant MultisigIncomingTxModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Status:
                return QString("Status");
            case Date:
                return QString("Date");
            case Signatures:
                return QString("Signatures");
            case SpendsOutputs:
                return QString("Spends Outputs");
            case TxCount:
                return QString("Tx Count");
            case TxId:
                return QString("Txid");
            case PrefixHash:
                return QString("Prefix hash");
            case Amount:
                return QString("Amount");
        }
    }
    return QVariant();
}


QVariant MultisigIncomingTxModel::getMessageInfo(const TxProposal &msg, int column, int role) const
{
    switch (column)
    {
        case Status: {
            switch (msg.status) {
                case TxProposal::Status::Completed:
                    return "Complete";
                case TxProposal::Status::Cant_Sign:
                    return "Can't sign";
                case TxProposal::Status::Pending:
                    return "Pending";
                case TxProposal::Status::Expired:
                    return "Expired";
                case TxProposal::Status::Frozen:
                    return "Frozen";
                case TxProposal::Status::Signed:
                    return "Signed";
                case TxProposal::Status::Double_Spend:
                    return "Double spend";
            }
        }
        case Date:
            return msg.timestamp.toString(QString("%1 %2 ").arg(conf()->get(Config::dateFormat).toString(),
                                                                conf()->get(Config::timeFormat).toString()));
        case Signatures:
            return msg.numSignatures;
        case MessageID:
            return msg.messageId;
        case SpendsOutputs:
            return msg.spendsOutputs.join(", ");
        case TxCount:
            return msg.txCount;
        case TxId:
            return msg.txId.mid(0, 8);
        case PrefixHash:
            return msg.prefixHash.mid(0, 8);
        case Amount:
            return WalletManager::displayAmount(msg.balanceDelta);
        default:
        {
            qCritical() << "Unimplemented role";
            return QVariant();
        }
    }
}

