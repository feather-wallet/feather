// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QtCore>
#include <QProcess>
#include "utils/childproc.h"

ChildProcess::ChildProcess(QObject* parent) {}
ChildProcess::~ChildProcess() {}

void ChildProcess::setupChildProcess() {
#if defined(HAVE_SYS_PRCTL_H) && defined(Q_OS_UNIX)
    // https://smackerelofopinion.blogspot.com/2015/11/using-prsetpdeathsig-to-reap-child.html
    ::prctl(PR_SET_PDEATHSIG, SIGKILL);
#endif
}