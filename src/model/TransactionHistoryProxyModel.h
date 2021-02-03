// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TRANSACTIONHISTORYPROXYMODEL_H
#define FEATHER_TRANSACTIONHISTORYPROXYMODEL_H

#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/Wallet.h"
#include "libwalletqt/TransactionHistory.h"

#include <QSortFilterProxyModel>

class TransactionHistoryProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit TransactionHistoryProxyModel(Wallet *wallet, QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public slots:
    void setSearchFilter(const QString& searchString){
        m_searchRegExp.setPattern(searchString);
        invalidateFilter();
    }

private:
    Wallet *m_wallet;
    TransactionHistory *m_history;

    QRegExp m_searchRegExp;
};

#endif //FEATHER_TRANSACTIONHISTORYPROXYMODEL_H
