// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TOR_H
#define FEATHER_TOR_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QtCore>
#include <QRegExp>
#include <QtNetwork>
#include <QApplication>
#include <QMainWindow>
#include "utils/childproc.h"

class Tor : public QObject
{
Q_OBJECT

public:
    explicit Tor(AppContext *ctx, QObject *parent = nullptr);

    void start();
    void stop();
    bool unpackBins();
    QString getVersion();
    networkPeer getPeerFromConfig(const QString &path);

    bool torConnected = false;
    bool localTor = false;
    QString torDir;
    QString torPath;
    QString torDataPath;

    static QString torHost;
    static quint16 torPort;

    QString torLogs;
    QString errorMsg = "";

signals:
    void connectionStateChanged(bool connected);
    void startupFailure(QString reason);
    void logsUpdated();

private slots:
    void stateChanged(QProcess::ProcessState);
    void handleProcessOutput();
    void handleProcessError(QProcess::ProcessError error);
    void checkConnection();

private:
    void setConnectionState(bool connected);

    ChildProcess m_process;
    AppContext *m_ctx;
    int m_restarts = 0;
    bool m_stopRetries = false;
    QTimer *m_checkConnectionTimer;
};

class AppContext;  // forward declaration

#endif //FEATHER_TOR_H