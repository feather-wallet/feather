////
//// Created by user on 1/9/24.
////
//
#ifndef FEATHER_MULTISIGINCOMINGTXMODEL_H
#define FEATHER_MULTISIGINCOMINGTXMODEL_H

#include <wallet/api/wallet2_api.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QIcon>

class MultisigMessageStore;
class MultisigMessage;
class TxProposal;

class MultisigIncomingTxModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Status,
        Date,
        MessageID,
        Signatures,
        SpendsOutputs,
        TxCount,
        TxId,
        PrefixHash,
        Amount,
        COUNT
    };

    explicit MultisigIncomingTxModel(QObject *parent, MultisigMessageStore *store);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void startReset();
    void endReset();

private:
    QVariant getMessageInfo(const TxProposal &msg, int column, int role) const;

    MultisigMessageStore *m_store;
};


#endif //FEATHER_MULTISIGINCOMINGTXMODEL_H
