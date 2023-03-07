// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef SUBADDRESSMODEL_H
#define SUBADDRESSMODEL_H

#include <wallet/api/wallet2_api.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QDebug>

class Subaddress;

class SubaddressModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
         Index = 0,
         Address,
         Label,
         isUsed,
         COUNT
    };

    explicit SubaddressModel(QObject *parent, Subaddress *subaddress);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    bool isShowFullAddresses() const;
    void setShowFullAddresses(bool show);

    Monero::SubaddressRow* entryFromIndex(const QModelIndex &index) const;

    void setCurrentSubaddressAccount(quint32 accountIndex);
    int unusedLookahead() const;

public slots:
    void startReset();
    void endReset();

private:
    Subaddress *m_subaddress;
    QVariant parseSubaddressRow(const Monero::SubaddressRow &subaddress, const QModelIndex &index, int role) const;

    bool m_showFullAddresses;
    quint32 m_currentSubaddressAccount;
};

#endif // SUBADDRESSMODEL_H
