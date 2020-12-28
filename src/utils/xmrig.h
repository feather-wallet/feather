// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_XMRIG_H
#define FEATHER_XMRIG_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QRegExp>
#include <QtNetwork>
#include <QApplication>
#include <QMainWindow>

#include "utils/childproc.h"

class XmRig : public QObject
{
Q_OBJECT

public:
    explicit XmRig(const QString &configDir, QObject *parent = nullptr);
    void prepare();

    void start(const QString &path, int threads, const QString &address, const QString &username, const QString &password, bool tor = false, bool tls = true);
    void stop();

    QString rigDir;
    QString rigPath;

signals:
    void error(const QString &msg);
    void output(const QByteArray &data);
    void hashrate(const QString &rate);

private slots:
    void stateChanged(QProcess::ProcessState);
    void handleProcessOutput();
    void handleProcessError(QProcess::ProcessError error);

private:
    ChildProcess m_process;
};

#endif //FEATHER_XMRIG_H
