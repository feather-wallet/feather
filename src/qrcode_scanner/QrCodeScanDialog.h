// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QCameraImageCapture>
#include <QTimer>
#include <QVideoFrame>

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
    void processAvailableImage(int id, const QVideoFrame &frame);
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);
    void displayCameraError();
    void takeImage();

    QScopedPointer<Ui::QrCodeScanDialog> ui;

    QScopedPointer<QCamera> m_camera;
    QScopedPointer<QCameraImageCapture> m_imageCapture;

    QrScanThread *m_thread;
    QTimer m_imageTimer;
    QList<QCameraInfo> m_cameras;
};

#endif //FEATHER_QRCODESCANDIALOG_H