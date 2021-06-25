// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.


#ifndef QRCODESCANNER_H_
#define QRCODESCANNER_H_

#include <QImage>
#include <QVideoFrame>
#include "QrScanThread.h"

class QVideoProbe;
class QCamera;

class QrCodeScanner : public QObject
{
Q_OBJECT

Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit QrCodeScanner(QObject *parent = nullptr);
    ~QrCodeScanner() override;
    void setSource(QCamera*);

    bool enabled() const;
    void setEnabled(bool enabled);

public slots:
    void processFrame(const QVideoFrame &frame);

signals:
    void enabledChanged();
    void decoded(int type, const QString &data);
    void decode(int type, const QString &data);
    void notifyError(const QString &error, bool warning = false);

protected:
    void timerEvent(QTimerEvent *);
    QrScanThread *m_thread;
    int m_processTimerId;
    int m_processInterval;
    int m_enabled;
    QVideoFrame m_curFrame;
    QVideoProbe *m_probe;
};

#endif
