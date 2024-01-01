// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef REDDITPROXYMODEL_H
#define REDDITPROXYMODEL_H

#include <QSortFilterProxyModel>

class RedditProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit RedditProxyModel(QObject* parent = nullptr);
};

#endif //REDDITPROXYMODEL_H
