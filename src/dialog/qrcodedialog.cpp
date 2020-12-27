// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "qrcodedialog.h"
#include "ui_qrcodedialog.h"
#include "qrcode/QrCode.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

QrCodeDialog::QrCodeDialog(QWidget *parent, const QrCode &qrCode, const QString &title)
        : QDialog(parent)
        , ui(new Ui::QrCodeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(title);

    m_pixmap = qrCode.toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio);
    ui->QrCode->setPixmap(m_pixmap);

    connect(ui->btn_CopyImage, &QPushButton::clicked, this, &QrCodeDialog::copyImage);
    connect(ui->btn_Save, &QPushButton::clicked, this, &QrCodeDialog::saveImage);
    connect(ui->btn_Close, &QPushButton::clicked, [this](){
        accept();
    });

    this->adjustSize();
}

QrCodeDialog::~QrCodeDialog()
{
    delete ui;
}

void QrCodeDialog::setQrCode(const QrCode &qrCode) {
    m_pixmap = qrCode.toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio);
    ui->QrCode->setPixmap(m_pixmap);
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