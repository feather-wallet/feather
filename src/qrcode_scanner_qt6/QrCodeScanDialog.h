// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QScopedPointer>
#include <QMediaCaptureSession>
#include <QTimer>

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
    void onDecoded(int type, const QString &data);
    void notifyError(const QString &msg);

private:
    void processCapturedImage(int requestId, const QImage& img);
    void displayCameraError();
    void takeImage();

    QScopedPointer<Ui::QrCodeScanDialog> ui;

    QrScanThread *m_thread;
    QImageCapture *m_imageCapture;
    QTimer m_imageTimer;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
};


#endif //FEATHER_QRCODESCANDIALOG_H
