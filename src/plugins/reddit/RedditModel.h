// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_REDDITMODEL_H
#define FEATHER_REDDITMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "RedditPost.h"

class RedditModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Title = 0,
        Author,
        Comments,
        COUNT
    };

    explicit RedditModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updatePosts(const QList<QSharedPointer<RedditPost>>& posts);

    QSharedPointer<RedditPost> post(int row);

private:
    QList<QSharedPointer<RedditPost>> m_posts;
};

#endif //FEATHER_REDDITMODEL_H
