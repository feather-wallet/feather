// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef _QRSCANTHREAD_H_
#define _QRSCANTHREAD_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QEvent>
#include <QVideoFrame>
#include <QCamera>
#include <zbar.h>

class QrScanThread : public QThread, public zbar::Image::Handler
{
    Q_OBJECT

public:
    QrScanThread(QObject *parent = nullptr);
    void addFrame(const QVideoFrame &frame);
    virtual void stop();

signals:
    void decoded(int type, const QString &data);
    void notifyError(const QString &error, bool warning = false);

protected:
    virtual void run();
    void processVideoFrame(const QVideoFrame &);
    void processQImage(const QImage &);
    void processZImage(zbar::Image &image);
    virtual void image_callback(zbar::Image &image);
    bool zimageFromQImage(const QImage&, zbar::Image &);

private:
    zbar::ImageScanner m_scanner;
    QSharedPointer<zbar::Image> m_image;
    bool m_running;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QList<QVideoFrame> m_queue;
};
#endif