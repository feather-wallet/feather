// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_QRCODESCANDIALOG_H
#define FEATHER_QRCODESCANDIALOG_H

#include <QDialog>
#include <QCamera>

#include "QrCodeScanner.h"

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
    Ui::QrCodeScanDialog *ui;

    QList<QCameraInfo> m_cameras;
    QCamera *m_camera = nullptr;
    QrCodeScanner *m_scanner;
};

#endif //FEATHER_QRCODESCANDIALOG_H