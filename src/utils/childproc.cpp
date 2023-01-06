// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "utils/childproc.h"

ChildProcess::ChildProcess(QObject* parent) {}
ChildProcess::~ChildProcess() {}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void ChildProcess::setupChildProcess() {
#if defined(HAVE_SYS_PRCTL_H) && defined(Q_OS_UNIX)
    // https://smackerelofopinion.blogspot.com/2015/11/using-prsetpdeathsig-to-reap-child.html
    ::prctl(PR_SET_PDEATHSIG, SIGKILL);
#endif
}
#endif