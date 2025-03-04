// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PaymentRequestDialog.h"
#include "ui_PaymentRequestDialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpressionValidator>

#include "WalletManager.h"
#include "utils/AppData.h"
#include "utils/config.h"
#include "utils/Utils.h"

PaymentRequestDialog::PaymentRequestDialog(QWidget *parent, Wallet *wallet, QString address)
    : WindowModalDialog(parent)
    , ui(new Ui::PaymentRequestDialog)
    , m_wallet(wallet)
    , m_address(std::move(address))
{
    ui->setupUi(this);

    QString amount_rx = R"(^\d{0,8}[\.]\d{0,12}|(all)$)";
    QRegularExpression rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->line_amount->setValidator(validator);

    connect(ui->line_amount, &QLineEdit::textEdited, [this] (const QString &text){
        this->calculateFiat();
        this->updatePaymentRequest();
    });
    connect(ui->line_amountFiat, &QLineEdit::textEdited, [this](const QString &text) {
        this->calculateCrypto();
        this->updatePaymentRequest();
    });

    connect(ui->line_description, &QLineEdit::textChanged, this, &PaymentRequestDialog::updatePaymentRequest);
    connect(ui->line_recipient, &QLineEdit::textChanged, this, &PaymentRequestDialog::updatePaymentRequest);

    connect(ui->btn_copyLink, &QPushButton::clicked, this, &PaymentRequestDialog::copyLink);
    connect(ui->btn_copyImage, &QPushButton::clicked, this, &PaymentRequestDialog::copyImage);
    connect(ui->btn_saveImage, &QPushButton::clicked, this, &PaymentRequestDialog::saveImage);

    QString preferredFiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
    QStringList fiatSymbols = conf()->get(Config::fiatSymbols).toStringList();
    if (fiatSymbols.contains(preferredFiatCurrency)) {
        fiatSymbols.removeAll(preferredFiatCurrency);
    }

    ui->comboCurrency->addItem(preferredFiatCurrency);
    ui->comboCurrency->addItems(fiatSymbols);
    ui->comboCurrency->setCurrentIndex(0);
    connect(ui->comboCurrency, &QComboBox::currentIndexChanged, [this] (int index){
        calculateFiat();
    });

    if (conf()->get(Config::disableWebsocket).toBool()) {
        ui->frame_fiat->hide();
    }

    this->updatePaymentRequest();

    ui->line_amount->setFocus();

    this->adjustSize();
}

void PaymentRequestDialog::updatePaymentRequest() {
    QString description = ui->line_description->text();
    QString recipient = ui->line_recipient->text();
    quint64 amount = WalletManager::amountFromString(ui->line_amount->text());

    QString uri = m_wallet->make_uri(m_address, amount, description, recipient);

    ui->line_paymentRequestUri->setText(uri);
    ui->line_paymentRequestUri->setCursorPosition(0);

    // TODO: memory leak, cba to refactor now
    m_qrCode = new QrCode(uri, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM);
    if (m_qrCode->isValid()) {
        ui->qrWidget->setQrCode(m_qrCode);
    }
}

void PaymentRequestDialog::calculateCrypto() {
    QString fiatCurrency = ui->comboCurrency->currentText();
    QString fiatAmount = ui->line_amountFiat->text();

    double cryptoAmount = appData()->prices.convert(fiatCurrency, "XMR", fiatAmount.toDouble());
    ui->line_amount->setText(QString::number(cryptoAmount, 'f', 10));
}

void PaymentRequestDialog::calculateFiat() {
    QString fiatCurrency = ui->comboCurrency->currentText();
    QString cryptoAmount = ui->line_amount->text();

    double fiatAmount = appData()->prices.convert("XMR", fiatCurrency, cryptoAmount.toDouble());
    ui->line_amountFiat->setText(QString::number(fiatAmount, 'f', 2));
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
