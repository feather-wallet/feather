// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_COINSPROXYMODEL_H
#define FEATHER_COINSPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "libwalletqt/Coins.h"

class CoinsProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit CoinsProxyModel(QObject* parent, Coins *coins);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

public slots:
    void setSearchFilter(const QString &searchString);
    void setShowSpent(bool showSpent);

private:
    Coins *m_coins;
    bool m_showSpent = false;
    QRegularExpression m_searchRegExp;
};

#endif //FEATHER_COINSPROXYMODEL_H
