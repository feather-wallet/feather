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


XMRig::XMRig(QObject *parent) : QObject(parent)
{
    qDebug() << "Using embedded tor instance";
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &XMRig::handleProcessOutput);
    connect(&m_process, &QProcess::errorOccurred, this, &XMRig::handleProcessError);
    connect(&m_process, &QProcess::stateChanged, this, &XMRig::stateChanged);
}

void XMRig::stop() {
    if(m_process.state() == QProcess::Running)
        m_process.kill();
}

void XMRig::terminate() {
    if(m_process.state() == QProcess::Running)
        m_process.terminate();
}

void XMRig::start(unsigned int threads, const QString &pool_name, const QString &receiving_address, bool tor) {
    auto state = m_process.state();
    if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
        emit error("Can't start XMRig, already running or starting");
        return;
    }

    auto path = config()->get(Config::xmrigPath).toString();
    if(path.isEmpty()) {
        emit error("Please set path to XMRig binary before starting.");
        return;
    }

    if(!Utils::fileExists(path)) {
        emit error("Path to XMRig binary invalid; file does not exist.");
        return;
    }

    QStringList arguments;
    arguments << "-o" << pool_name;
    arguments << "-a" << "rx/0";
    arguments << "-u" << receiving_address;
    arguments << "-p" << "featherwallet";
    arguments << "--no-color";
    arguments << "-t" << QString::number(threads);
    if(tor)
        arguments << "-x" << QString("%1:%2").arg(Tor::torHost).arg(Tor::torPort);

    QString cmd = QString("%1 %2").arg(path, arguments.join(" "));
    emit output(cmd.toUtf8());
    m_process.start(path, arguments);
}

void XMRig::stateChanged(QProcess::ProcessState state) {
    if(state == QProcess::ProcessState::Running)
        emit output("XMRig started");
    else if (state == QProcess::ProcessState::NotRunning)
        emit output("XMRig stopped");
}

void XMRig::handleProcessOutput() {
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

void XMRig::handleProcessError(QProcess::ProcessError err) {
    if (err == QProcess::ProcessError::Crashed)
        emit error("XMRig crashed or killed");
    else if (err == QProcess::ProcessError::FailedToStart) {
        auto path = config()->get(Config::xmrigPath).toString();
        emit error(QString("XMRig binary failed to start: %1").arg(path));
    }
}

