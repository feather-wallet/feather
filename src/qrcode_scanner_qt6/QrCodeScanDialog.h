// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QScopedPointer>
#include <QMediaCaptureSession>

namespace Ui {
    class QrCodeScanDialog;
}

class QrCodeScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrCodeScanDialog(QWidget *parent);
    ~QrCodeScanDialog() override;

private:
    QScopedPointer<Ui::QrCodeScanDialog> ui;

    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
};


#endif //FEATHER_QRCODESCANDIALOG_H
