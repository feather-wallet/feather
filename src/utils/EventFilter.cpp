// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "EventFilter.h"

#include <QKeyEvent>
#include <QDebug>

EventFilter::EventFilter(QObject *parent)
    : QObject(parent)
{}

bool EventFilter::eventFilter(QObject *obj, QEvent *ev) {
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::MouseButtonRelease) {
        emit userActivity();
    }

    return QObject::eventFilter(obj, ev);
}