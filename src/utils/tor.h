// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TOR_H
#define FEATHER_TOR_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QRegExp>
#include <QtNetwork>
#include "utils/childproc.h"

struct TorVersion
{
    explicit TorVersion(int major=0, int minor=0, int patch=0, int release=0)
        : patch(patch), release(release)
    {
        this->major = major;
        this->minor = minor;
    }

    friend bool operator== (const TorVersion &v1, const TorVersion &v2) {
        return (v1.major == v2.major &&
                v1.minor == v2.minor &&
                v1.patch == v2.patch &&
                v1.release == v2.release);
    }

    friend bool operator!= (const TorVersion &v1, const TorVersion &v2) {
        return !(v1 == v2);
    }

    friend bool operator> (const TorVersion &v1, const TorVersion &v2) {
        if (v1.major != v2.major)
            return v1.major > v2.major;
        if (v1.minor != v2.minor)
            return v1.minor > v2.minor;
        if (v1.patch != v2.patch)
            return v1.patch > v2.patch;
        if (v1.release != v2.release)
            return v1.release > v2.release;
        return false;
    }

    friend bool operator< (const TorVersion &v1, const TorVersion &v2) {
        if (v1 == v2)
            return false;
        return !(v1 > v2);
    }

    QString toString() {
        return QString("%1.%2.%3.%4").arg(QString::number(major), QString::number(minor),
                                          QString::number(patch), QString::number(release));
    }

    int major;
    int minor;
    int patch;
    int release;
};

class Tor : public QObject
{
Q_OBJECT

public:
    explicit Tor(AppContext *ctx, QObject *parent = nullptr);

    void start();
    void stop();
    bool unpackBins();
    TorVersion getVersion(const QString &fileName);
    TorVersion stringToVersion(const QString &version);

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