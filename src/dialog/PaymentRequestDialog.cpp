// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PaymentRequestDialog.h"
#include "ui_PaymentRequestDialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExpValidator>

#include "WalletManager.h"

PaymentRequestDialog::PaymentRequestDialog(QWidget *parent, QSharedPointer<AppContext> ctx, QString address)
    : QDialog(parent)
    , ui(new Ui::PaymentRequestDialog)
    , m_ctx(std::move(ctx))
    , m_address(std::move(address))
{
    ui->setupUi(this);

    QString amount_rx = R"(^\d{0,8}[\.]\d{0,12}|(all)$)";
    QRegExp rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->line_amountXMR->setValidator(validator);

    connect(ui->line_amountXMR, &QLineEdit::textChanged, this, &PaymentRequestDialog::updatePaymentRequest);
    connect(ui->line_description, &QLineEdit::textChanged, this, &PaymentRequestDialog::updatePaymentRequest);
    connect(ui->line_recipient, &QLineEdit::textChanged, this, &PaymentRequestDialog::updatePaymentRequest);

    connect(ui->btn_copyLink, &QPushButton::clicked, this, &PaymentRequestDialog::copyLink);
    connect(ui->btn_copyImage, &QPushButton::clicked, this, &PaymentRequestDialog::copyImage);
    connect(ui->btn_saveImage, &QPushButton::clicked, this, &PaymentRequestDialog::saveImage);

    this->updatePaymentRequest();

    ui->line_amountXMR->setFocus();

    this->adjustSize();
}

void PaymentRequestDialog::updatePaymentRequest() {
    QString description = ui->line_description->text();
    QString recipient = ui->line_recipient->text();
    quint64 amount = WalletManager::amountFromString(ui->line_amountXMR->text());

    QString uri = m_ctx->wallet->make_uri(m_address, amount, description, recipient);

    ui->line_paymentRequestUri->setText(uri);
    ui->line_paymentRequestUri->setCursorPosition(0);

    // TODO: memory leak, cba to refactor now
    m_qrCode = new QrCode(uri, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM);
    if (m_qrCode->isValid()) {
        ui->qrWidget->setQrCode(m_qrCode);
    }
}

void PaymentRequestDialog::copyLink() {
    Utils::copyToClipboard(ui->line_paymentRequestUri->text());
    QMessageBox::information(this, "Information", "Payment request link copied to clipboard.");
}

void PaymentRequestDialog::copyImage() {
    QApplication::clipboard()->setPixmap(m_qrCode->toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio));
    QMessageBox::information(this, "Information", "QR code copied to clipboard.");
}

void PaymentRequestDialog::saveImage() {
    QString filename = QFileDialog::getSaveFileName(this, "Select where to save file", QDir::current().filePath("qrcode.png"));
    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    m_qrCode->toPixmap(1).scaled(500, 500, Qt::KeepAspectRatio).save(&file, "PNG");
    QMessageBox::information(this, "Information", "QR code saved to file");
}

PaymentRequestDialog::~PaymentRequestDialog() = default;