// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_QRCODEDIALOG_H
#define FEATHER_QRCODEDIALOG_H

#include <QDialog>

#include "components.h"
#include "qrcode/QrCode.h"
#include "widgets/QrCodeWidget.h"

namespace Ui {
    class QrCodeDialog;
}

class QrCodeDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit QrCodeDialog(QWidget *parent, QrCode *qrCode, const QString &title = "Qr Code");
    ~QrCodeDialog() override;

private:
    void copyImage();
    void saveImage();

    QScopedPointer<Ui::QrCodeDialog> ui;
    QPixmap m_pixmap;
};

#endif //FEATHER_QRCODEDIALOG_H
