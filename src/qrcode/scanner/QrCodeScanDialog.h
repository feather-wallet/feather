// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QScopedPointer>
#include <QMediaCaptureSession>
#include <QTimer>
#include <QVideoSink>

#include "QrScanThread.h"

namespace Ui {
    class QrCodeScanDialog;
}

class QrCodeScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrCodeScanDialog(QWidget *parent);
    ~QrCodeScanDialog() override;

    QString decodedString = "";

private slots:
    void onCameraSwitched(int index);
    void onDecoded(const QString &data);

private:
    QImage videoFrameToImage(const QVideoFrame &videoFrame);
    void handleFrameCaptured(const QVideoFrame &videoFrame);

    QScopedPointer<Ui::QrCodeScanDialog> ui;

    QrScanThread *m_thread;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
    QVideoSink m_sink;
};


#endif //FEATHER_QRCODESCANDIALOG_H
