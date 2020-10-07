// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_REDDITPOST_H
#define FEATHER_REDDITPOST_H

#include <QString>

struct RedditPost {
    RedditPost(const QString &title, const QString &author, const QString &url, int comments) : title(title), author(author), url(url), comments(comments){};

    QString title;
    QString author;
    QString url;
    int comments;
};

#endif //FEATHER_REDDITPOST_H
