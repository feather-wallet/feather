// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_REDDITPOST_H
#define FEATHER_REDDITPOST_H

#include <QString>

struct RedditPost {
    RedditPost(const QString &title, const QString &author, const QString &permalink, int comments)
            : title(title), author(author), permalink(permalink), comments(comments){};

    QString title;
    QString author;
    QString permalink;
    int comments;
};

#endif //FEATHER_REDDITPOST_H
