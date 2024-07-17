//
// Created by user on 1/3/24.
//

#ifndef FEATHER_MULTISIGMESSAGEMODEL_H
#define FEATHER_MULTISIGMESSAGEMODEL_H

#include <wallet/api/wallet2_api.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QIcon>

class MultisigMessageStore;
class MultisigMessage;

class MultisigMessageModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Id = 0,
        Type,
        Direction,
        Created,
        Modified,
        Sent,
        Signer,
        Hash,
        State,
        WalletHeight,
        Round,
        SignatureCount,
        TransportId,
        COUNT
    };

    explicit MultisigMessageModel(QObject *parent, MultisigMessageStore *store);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void startReset();
    void endReset();

private:
    QVariant getMessageInfo(const MultisigMessage &msg, int column, int role) const;

    MultisigMessageStore *m_store;
};

#endif //FEATHER_MULTISIGMESSAGEMODEL_H
