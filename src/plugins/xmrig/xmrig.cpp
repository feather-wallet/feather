// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include <QScreen>
#include <QDesktopServices>

#include "utils/config.h"
#include "utils/Utils.h"
#include "xmrig.h"
#include "utils/TorManager.h"

XmRig::XmRig(const QString &configDir, QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &XmRig::handleProcessOutput);
    connect(m_process, &QProcess::errorOccurred, this, &XmRig::handleProcessError);
    connect(m_process, &QProcess::stateChanged, this, &XmRig::onStateChanged);
}

void XmRig::stop() {
    qDebug() << m_process->processId();
    if (m_process->state() == QProcess::Running) {
#if defined(Q_OS_WIN)
        m_process->kill(); // https://doc.qt.io/qt-5/qprocess.html#terminate
#elif defined(Q_OS_LINUX)
        if (m_elevated) {
            m_killProcess.start("pkexec", QStringList() << "kill" << QString::number(m_process->processId()));
            return;
        }
#endif
        m_process->terminate();
    }
}

void XmRig::start(const QString &path, int threads, const QString &address, const QString &username,
                  const QString &password, bool tor, bool tls, bool elevated, bool solo, const QStringList &extraOptions)
{
    m_elevated = elevated;

    auto state = m_process->state();
    if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
        emit error("Can't start XMRig, already running or starting");
        return;
    }

    if (path.isEmpty()) {
        emit error("XmRig->Start path parameter missing.");
        return;
    }

    if (!Utils::fileExists(path)) {
        emit error(QString("Path to XMRig binary invalid; file does not exist: %1").arg(path));
        return;
    }

    QStringList arguments;
    if (m_elevated) {
        arguments << path;
    }
    arguments << "-o" << address;
    arguments << "-u" << username;
    if (!password.isEmpty()) {
        arguments << "-p" << password;
    }
    arguments << "--no-color";
    arguments << "-t" << QString::number(threads);
    if (tor) {
        QString host = conf()->get(Config::socks5Host).toString();
        QString port = conf()->get(Config::socks5Port).toString();
        if (!torManager()->isLocalTor()) {
            host = torManager()->featherTorHost;
            port = QString::number(torManager()->featherTorPort);
        }
        arguments << "-x" << QString("%1:%2").arg(host, port);
    }
    if (tls) {
        arguments << "--tls";
    }
    if (solo) {
        arguments << "--daemon";
    }
    arguments += extraOptions;

    QString cmd = QString("%1 %2").arg(path, arguments.join(" "));
    emit output(cmd.toUtf8());

    if (m_elevated) {
        m_process->start("pkexec", arguments);
    } else {
        m_process->start(path, arguments);
    }
}

void XmRig::onStateChanged(QProcess::ProcessState state) {
    emit stateChanged(state);

    if (state == QProcess::ProcessState::Running) {
        emit output("XMRig started");
    }

    else if (state == QProcess::ProcessState::NotRunning) {
        emit output("XMRig stopped");
    }
}

void XmRig::handleProcessOutput() {
    QByteArray _output = m_process->readAllStandardOutput();
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
        auto path = conf()->get(Config::xmrigPath).toString();
        emit error(QString("XMRig binary failed to start: %1").arg(path));
    }
}
