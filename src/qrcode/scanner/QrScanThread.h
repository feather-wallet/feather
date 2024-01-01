// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef QRSCANTHREAD_H_
#define QRSCANTHREAD_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QEvent>
#include <QCamera>

#include <ZXing/ReadBarcode.h>

#include "qrcode/utils/QrCodeUtils.h"

class QrScanThread : public QThread
{
    Q_OBJECT

public:
    explicit QrScanThread(QObject *parent = nullptr);
    void addImage(const QImage &img);
    
    virtual void stop();
    virtual void start();
    
signals:
    void decoded(const QString &data);

protected:
    void run() override;
    void processQImage(const QImage &);

private:
    bool m_running;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QList<QImage> m_queue;
};
#endif