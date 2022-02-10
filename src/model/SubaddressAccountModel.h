// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#ifndef SUBADDRESSACCOUNTMODEL_H
#define SUBADDRESSACCOUNTMODEL_H

#include "Subaddress.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class SubaddressAccount;

class SubaddressAccountModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        Number,
        Address,
        Label,
        Balance,
        UnlockedBalance,
        COUNT
    };

    SubaddressAccountModel(QObject *parent, SubaddressAccount *subaddressAccount);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Monero::SubaddressAccountRow* entryFromIndex(const QModelIndex &index) const;

public slots:
    void startReset();
    void endReset();

private:
    QVariant parseSubaddressAccountRow(const Monero::SubaddressAccountRow &row, const QModelIndex &index, int role) const;

    SubaddressAccount *m_subaddressAccount;
};

class SubaddressAccountProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SubaddressAccountProxyModel(QObject *parent = nullptr);
};

#endif // SUBADDRESSACCOUNTMODEL_H
