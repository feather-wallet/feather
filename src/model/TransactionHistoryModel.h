// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef TRANSACTIONHISTORYMODEL_H
#define TRANSACTIONHISTORYMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include "appcontext.h"

class TransactionHistory;
class TransactionInfo;

/**
 * @brief The TransactionHistoryModel class - read-only table model for Transaction History
 */

class TransactionHistoryModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(TransactionHistory * transactionHistory READ transactionHistory WRITE setTransactionHistory NOTIFY transactionHistoryChanged)

public:
    enum Column
    {
        Date = 0,
        TxID,
        Description,
        Amount,
        FiatAmount,
        COUNT
    };

    explicit TransactionHistoryModel(QObject * parent = nullptr);
    void setTransactionHistory(TransactionHistory * th);
    TransactionHistory * transactionHistory() const;
    TransactionInfo* entryFromIndex(const QModelIndex& index) const;

    QString preferredFiatSymbol = "USD";
    int amountPrecision = 4;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

signals:
    void transactionHistoryChanged();

private:
    QVariant parseTransactionInfo(const TransactionInfo &tInfo, int column, int role) const;

    TransactionHistory * m_transactionHistory;
    QIcon m_unconfirmedTx;
    QIcon m_warning;
    QIcon m_clock1;
    QIcon m_clock2;
    QIcon m_clock3;
    QIcon m_clock4;
    QIcon m_clock5;
    QIcon m_confirmedTx;
};

#endif // TRANSACTIONHISTORYMODEL_H
