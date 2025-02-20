// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_REVUOITEM_H
#define FEATHER_REVUOITEM_H

#include <QString>
#include <QStringList>

struct RevuoItem : QObject
{
    Q_OBJECT

public:
    explicit RevuoItem(QObject *parent)
        : QObject(parent) {};

    QString title;
    QString url;
    QStringList newsbytes;
    QList<QPair<QString, QString>> events;
};

#endif //FEATHER_REVUOITEM_H
