// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

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
    explicit QrCodeDialog(QWidget *parent, const QString &text, const QString &title = "Qr Code");
    ~QrCodeDialog() override;

private:
    void copyImage();
    void saveImage();

    Ui::QrCodeDialog *ui;
    QrCode *m_qrc;
    QPixmap m_pixmap;
};


#endif //FEATHER_QRCODEDIALOG_H
