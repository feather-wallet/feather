// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicFundDialog.h"
#include "ui_AtomicFundDialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

AtomicFundDialog::AtomicFundDialog(QWidget *parent, const QString &title, const QString &btc_address)
        : WindowModalDialog(parent)
        , ui(new Ui::AtomicFundDialog)
        , address(btc_address)
        , qrCode(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH)
{
    ui->setupUi(this);
    this->setWindowTitle(title);

    ui->qrWidget->setQrCode(&qrCode);
    m_pixmap = qrCode.toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio);
    connect(ui->btn_CopyAddress, &QPushButton::clicked, this, &AtomicFundDialog::copyAddress);
    connect(ui->btn_CopyImage, &QPushButton::clicked, this, &AtomicFundDialog::copyImage);
    connect(ui->btn_Save, &QPushButton::clicked, this, &AtomicFundDialog::saveImage);
    connect(ui->btn_Close, &QPushButton::clicked, [this](){
        accept();
    });

    this->resize(500, 500);
}

void AtomicFundDialog::copyImage() {
    QApplication::clipboard()->setPixmap(m_pixmap);
    QMessageBox::information(this, "Information", "QR code copied to clipboard");
}

void AtomicFundDialog::saveImage() {
    QString filename = QFileDialog::getSaveFileName(this, "Select where to save file", QDir::current().filePath("qrcode.png"));
    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    m_pixmap.save(&file, "PNG");
    QMessageBox::information(this, "Information", "QR code saved to file");
}

void AtomicFundDialog::copyAddress(){
    QApplication::clipboard()->setText(address);
    QMessageBox::information(this, "Information", "BTC deposit address copied to clipboard");
}



AtomicFundDialog::~AtomicFundDialog() {
    emit cleanProcs();
}