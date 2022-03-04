// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_EVENTFILTER_H
#define FEATHER_EVENTFILTER_H

#include <QObject>

class EventFilter : public QObject
{
Q_OBJECT

public:
    explicit EventFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);


signals:
    void userActivity();
};


#endif //FEATHER_EVENTFILTER_H
