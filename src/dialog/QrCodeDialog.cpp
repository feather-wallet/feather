// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "QrCodeDialog.h"
#include "ui_QrCodeDialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

QrCodeDialog::QrCodeDialog(QWidget *parent, QrCode *qrCode, const QString &title)
        : WindowModalDialog(parent)
        , ui(new Ui::QrCodeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(title);

    ui->qrWidget->setQrCode(qrCode);

    m_pixmap = qrCode->toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio);

    connect(ui->btn_CopyImage, &QPushButton::clicked, this, &QrCodeDialog::copyImage);
    connect(ui->btn_Save, &QPushButton::clicked, this, &QrCodeDialog::saveImage);
    connect(ui->btn_Close, &QPushButton::clicked, [this](){
        accept();
    });

    this->resize(500, 500);
}

void QrCodeDialog::copyImage() {
    QApplication::clipboard()->setPixmap(m_pixmap);
    QMessageBox::information(this, "Information", "QR code copied to clipboard");
}

void QrCodeDialog::saveImage() {
    QString filename = QFileDialog::getSaveFileName(this, "Select where to save file", QDir::current().filePath("qrcode.png"));
    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    m_pixmap.save(&file, "PNG");
    QMessageBox::information(this, "Information", "QR code saved to file");
}

QrCodeDialog::~QrCodeDialog() = default;