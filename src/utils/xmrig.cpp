// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QtCore>
#include <QScreen>
#include <QDesktopWidget>
#include <QProcess>
#include <QDesktopServices>

#include "utils/utils.h"
#include "utils/xmrig.h"
#include "appcontext.h"

XmRig::XmRig(const QString &configDir, QObject *parent) : QObject(parent) {
    this->rigDir = QDir(configDir).filePath("xmrig");
}

void XmRig::prepare() {
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &XmRig::handleProcessOutput);
    connect(&m_process, &QProcess::errorOccurred, this, &XmRig::handleProcessError);
    connect(&m_process, &QProcess::stateChanged, this, &XmRig::stateChanged);
}

void XmRig::stop() {
    if(m_process.state() == QProcess::Running) {
#if defined(Q_OS_WIN)
        m_process.kill(); // https://doc.qt.io/qt-5/qprocess.html#terminate
#else
        m_process.terminate();
#endif
    }
}

void XmRig::start(const QString &path,
                  unsigned int threads,
                  const QString &address,
                  const QString &username,
                  const QString &password,
                  bool tor, bool tls) {
    auto state = m_process.state();
    if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
        emit error("Can't start XMRig, already running or starting");
        return;
    }

    if(path.isEmpty()) {
        emit error("XmRig->Start path parameter missing.");
        return;
    }

    if(!Utils::fileExists(path)) {
        emit error(QString("Path to XMRig binary invalid; file does not exist: %1").arg(path));
        return;
    }

    QStringList arguments;
    arguments << "-o" << address;
    arguments << "-a" << "rx/0";
    arguments << "-u" << username;
    if(!password.isEmpty())
        arguments << "-p" << password;
    arguments << "--no-color";
    arguments << "-t" << QString::number(threads);
    if(tor)
        arguments << "-x" << QString("%1:%2").arg(Tor::torHost).arg(Tor::torPort);
    if(tls)
        arguments << "--tls";
    arguments << "--donate-level" << "1";
    QString cmd = QString("%1 %2").arg(path, arguments.join(" "));
    emit output(cmd.toUtf8());
    m_process.start(path, arguments);
}

void XmRig::stateChanged(QProcess::ProcessState state) {
    if(state == QProcess::ProcessState::Running)
        emit output("XMRig started");
    else if (state == QProcess::ProcessState::NotRunning)
        emit output("XMRig stopped");
}

void XmRig::handleProcessOutput() {
    QByteArray _output = m_process.readAllStandardOutput();
    if(_output.contains("miner") && _output.contains("speed")) {
        // detect hashrate
        auto str = Utils::barrayToString(_output);
        auto spl = str.mid(str.indexOf("speed")).split(" ");
        auto rate = spl.at(2) + "H/s";
        qDebug() << "mining hashrate: " << rate;
        emit hashrate(rate);
    }

    emit output(_output);
}

void XmRig::handleProcessError(QProcess::ProcessError err) {
    if (err == QProcess::ProcessError::Crashed)
        emit error("XMRig crashed or killed");
    else if (err == QProcess::ProcessError::FailedToStart) {
        auto path = config()->get(Config::xmrigPath).toString();
        emit error(QString("XMRig binary failed to start: %1").arg(path));
    }
}
