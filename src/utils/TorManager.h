// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TOR_H
#define FEATHER_TOR_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QRegularExpression>
#include <QtNetwork>
#include "utils/childproc.h"
#include "utils/SemanticVersion.h"

class TorManager : public QObject
{
Q_OBJECT

public:
    explicit TorManager(QObject *parent = nullptr);

    void init();
    void start();
    void stop();
    bool unpackBins();
    bool isLocalTor();
    bool isStarted();
    SemanticVersion getVersion(const QString &fileName);

    static TorManager* instance();

    bool torConnected = false;

    QString featherTorHost = "127.0.0.1";
    quint16 featherTorPort = 19450;

    QString torDir;
    QString torPath;
    QString torDataPath;

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
    bool shouldStartTorDaemon();
    void setConnectionState(bool connected);

    static QPointer<TorManager> m_instance;

    ChildProcess m_process;
    int m_restarts = 0;
    bool m_stopRetries = false;
    bool m_localTor;
    bool m_started = false;
    QTimer *m_checkConnectionTimer;
};

inline TorManager* torManager()
{
    return TorManager::instance();
}

#endif //FEATHER_TOR_H