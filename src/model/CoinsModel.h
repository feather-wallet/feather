// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_COINSMODEL_H
#define FEATHER_COINSMODEL_H

#include <wallet/api/wallet2_api.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QIcon>

class Coins;
class CoinsInfo;

class CoinsModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        KeyImageKnown = 0,
        PubKey,
        OutputPoint,
        Address,
        AddressLabel,
        BlockHeight,
        SpentHeight,
        Amount,
        Spent,
        Frozen,
        COUNT
    };

    explicit CoinsModel(QObject *parent, Coins *coins);

    Coins * coins() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void startReset();
    void endReset();

private:
    QVariant parseTransactionInfo(const CoinsInfo &cInfo, int column, int role) const;

    Coins *m_coins;
    QIcon m_eye;
    QIcon m_eyeBlind;
};

#endif //FEATHER_COINSMODEL_H
