// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_XMRIG_H
#define FEATHER_XMRIG_H

#include <QProcess>

class XmRig : public QObject
{
Q_OBJECT

public:
    explicit XmRig(const QString &configDir, QObject *parent = nullptr);

    void start(const QString &path, int threads, const QString &address, const QString &username, const QString &password, bool tor = false, bool tls = true, bool elevated = false, bool solo = false, const QStringList &extraOptions = {});
    void stop();

signals:
    void error(const QString &msg);
    void output(const QByteArray &data);
    void hashrate(const QString &rate);
    void stateChanged(QProcess::ProcessState state);

private slots:
    void onStateChanged(QProcess::ProcessState);
    void handleProcessOutput();
    void handleProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process;
    QProcess m_killProcess;
    bool m_elevated;
};

#endif //FEATHER_XMRIG_H
