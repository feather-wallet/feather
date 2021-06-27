// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_QRCODEDIALOG_H
#define FEATHER_QRCODEDIALOG_H

#include <QDialog>

#include "qrcode/QrCode.h"

namespace Ui {
    class QrCodeDialog;
}

class QrCodeDialog : public QDialog
{
Q_OBJECT

public:
    explicit QrCodeDialog(QWidget *parent, const QrCode &qrCode, const QString &title = "Qr Code");
    ~QrCodeDialog() override;
    void setQrCode(const QrCode &qrCode);

private:
    void copyImage();
    void saveImage();

    QScopedPointer<Ui::QrCodeDialog> ui;
    QPixmap m_pixmap;
};


#endif //FEATHER_QRCODEDIALOG_H
