// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_REVUOITEM_H
#define FEATHER_REVUOITEM_H

#include <QString>
#include <QStringList>

struct RevuoItem {
    RevuoItem(const QString &title, const QString &url, const QStringList &newsbytes)
        : title(title), url(url), newsbytes(newsbytes){};

    QString title;
    QString url;
    QStringList newsbytes;
};

#endif //FEATHER_REVUOITEM_H
