// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_COINSPROXYMODEL_H
#define FEATHER_COINSPROXYMODEL_H

#include <QSortFilterProxyModel>

class CoinsProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit CoinsProxyModel(QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public slots:
    void setShowSpent(const bool showSpent){
        m_showSpent = showSpent;
        invalidateFilter();
    }

private:
    bool m_showSpent = false;
};

#endif //FEATHER_COINSPROXYMODEL_H
