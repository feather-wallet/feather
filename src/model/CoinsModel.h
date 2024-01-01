// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

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
        TxID,
        Address,
        Label,
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
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    CoinsInfo* entryFromIndex(const QModelIndex &index) const;

    void setCurrentSubaddressAccount(quint32 accountIndex);
    void setSelected(const QStringList &selected);

signals:
    void descriptionChanged();

public slots:
    void startReset();
    void endReset();

private:
    QVariant parseTransactionInfo(const CoinsInfo &cInfo, int column, int role) const;

    Coins *m_coins;
    quint32 m_currentSubaddressAccount;
    QSet<QString> m_selected;
};

#endif //FEATHER_COINSMODEL_H
