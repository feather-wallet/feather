// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_MACOS_SLEEP_H
#define FEATHER_MACOS_SLEEP_H

#include <QObject>

class MacSleepObserver : public QObject {
    Q_OBJECT

public:
    explicit MacSleepObserver(QObject *parent = nullptr);
    ~MacSleepObserver() override;

    void notifyWillSleep();
    void notifyDidWake();

signals:
    void willSleep();
    void didWake();

private:
    void *m_observer = nullptr;
};

#endif // FEATHER_MACOS_SLEEP_H
