// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_FPROCESS_H
#define FEATHER_FPROCESS_H

#include <QtCore>
#include <QProcess>

#if defined(HAVE_SYS_PRCTL_H) && defined(Q_OS_UNIX)
#include <signal.h>
#include <sys/prctl.h>
#endif

class ChildProcess : public QProcess {
    Q_OBJECT
public:
    explicit ChildProcess(QObject* parent = nullptr);
    ~ChildProcess();
protected:
    void setupChildProcess() override;
};


#endif //FEATHER_FPROCESS_H
