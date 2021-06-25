// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "QrCodeScanner.h"
#include <WalletManager.h>
#include <QVideoProbe>
#include <QCamera>

QrCodeScanner::QrCodeScanner(QObject *parent)
    : QObject(parent)
    , m_processTimerId(-1)
    , m_processInterval(750)
    , m_enabled(true)
{
    m_probe = new QVideoProbe(this);
    m_thread = new QrScanThread(this);
    m_thread->start();

    connect(m_thread, &QrScanThread::decoded, this, &QrCodeScanner::decoded);
    connect(m_thread, &QrScanThread::notifyError, this, &QrCodeScanner::notifyError);
    connect(m_probe, &QVideoProbe::videoFrameProbed, this, &QrCodeScanner::processFrame);
}

void QrCodeScanner::setSource(QCamera *camera)
{
    m_probe->setSource((QMediaObject *)camera);
}

void QrCodeScanner::processFrame(const QVideoFrame &frame)
{
    if (frame.isValid()){
        m_curFrame = frame;
    }
}

bool QrCodeScanner::enabled() const
{
    return m_enabled;
}

void QrCodeScanner::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if(!enabled && (m_processTimerId != -1) )
    {
        this->killTimer(m_processTimerId);
        m_processTimerId = -1;
    }
    else if (enabled && (m_processTimerId == -1) )
    {
        m_processTimerId = this->startTimer(m_processInterval);
    }
    emit enabledChanged();
}

void QrCodeScanner::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_processTimerId) {
        m_thread->addFrame(m_curFrame);
    }
}

QrCodeScanner::~QrCodeScanner()
{
    m_thread->stop();
    m_thread->quit();
    if (!m_thread->wait(5000))
    {
        m_thread->terminate();
        m_thread->wait();
    }
}
