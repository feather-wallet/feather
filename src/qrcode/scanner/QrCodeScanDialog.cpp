// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "QrCodeScanDialog.h"
#include "ui_QrCodeScanDialog.h"

QrCodeScanDialog::QrCodeScanDialog(QWidget *parent, bool scan_ur)
        : QDialog(parent)
        , ui(new Ui::QrCodeScanDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Scan QR code");
    
    ui->widget_scanner->startCapture(scan_ur);

    connect(ui->widget_scanner, &QrCodeScanWidget::finished, [this]{
        this->accept();
    });
}

QString QrCodeScanDialog::decodedString() {
    return ui->widget_scanner->decodedString;
}

QrCodeScanDialog::~QrCodeScanDialog()
{

}
