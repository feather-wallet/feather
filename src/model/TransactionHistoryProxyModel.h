// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TRANSACTIONHISTORYPROXYMODEL_H
#define FEATHER_TRANSACTIONHISTORYPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/Wallet.h"

class TransactionHistoryProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit TransactionHistoryProxyModel(Wallet *wallet, QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    TransactionHistory* history();

public slots:
    void setSearchFilter(const QString& searchString){
        m_searchRegExp.setPattern(searchString);
        invalidateFilter();
    }

private:
    Wallet *m_wallet;
    TransactionHistory *m_history;

    QRegularExpression m_searchRegExp;
};

#endif //FEATHER_TRANSACTIONHISTORYPROXYMODEL_H
