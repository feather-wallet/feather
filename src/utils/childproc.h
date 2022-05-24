// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_FPROCESS_H
#define FEATHER_FPROCESS_H

#include <QProcess>

#if defined(HAVE_SYS_PRCTL_H) && defined(Q_OS_UNIX)
#include <signal.h>
#include <sys/prctl.h>
#endif

class ChildProcess : public QProcess {
    Q_OBJECT
public:
    explicit ChildProcess(QObject* parent = nullptr);
    ~ChildProcess() override;
protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void setupChildProcess() override;
#endif
};


#endif //FEATHER_FPROCESS_H
