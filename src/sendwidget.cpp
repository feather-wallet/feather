// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.


#include <QMessageBox>
#include "sendwidget.h"
#include "widgets/ccswidget.h"
#include "mainwindow.h"
#include "ui_sendwidget.h"

SendWidget::SendWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SendWidget)
{
    ui->setupUi(this);

    QString amount_rx = R"(^\d{0,8}[\.,]\d{0,12}|(all)$)";
    QRegExp rx;
    rx.setPattern(amount_rx);
    QValidator *validator =  new QRegExpValidator(rx, this);
    ui->lineAmount->setValidator(validator);

    connect(ui->btnSend, &QPushButton::clicked, this, &SendWidget::sendClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &SendWidget::clearClicked);
    connect(ui->btnMax, &QPushButton::clicked, this, &SendWidget::btnMaxClicked);
    connect(ui->comboCurrencySelection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SendWidget::currencyComboChanged);
    connect(ui->lineAmount, &QLineEdit::textEdited, this, &SendWidget::amountEdited);
    connect(ui->lineAddress, &QLineEdit::textEdited, this, &SendWidget::addressEdited);
    connect(ui->btn_openAlias, &QPushButton::clicked, this, &SendWidget::aliasClicked);
    ui->label_xmrAmount->setText("");
    ui->label_xmrAmount->hide();
    ui->btn_openAlias->hide();
}

void SendWidget::currencyComboChanged(int index) {
    QString amount = ui->lineAmount->text();
    if(amount.isEmpty()) return;
    this->amountEdited(amount);
}

void SendWidget::addressEdited(const QString &text) {
    text.contains(".") ? ui->btn_openAlias->show() : ui->btn_openAlias->hide();
}

void SendWidget::amountEdited(const QString &text) {
    this->updateConversionLabel();
}

void SendWidget::fill(const CCSEntry &entry) {
    this->fill(entry.address, QString("CCS: %1").arg(entry.title), 0.0);
}

void SendWidget::fill(double amount) {
    ui->lineAmount->setText(QString::number(amount));
}

void SendWidget::fill(const QString &address, const QString& description){
    ui->lineDescription->setText(description);
    ui->lineAddress->setText(address);
    ui->lineAddress->setCursorPosition(0);
}

void SendWidget::fill(const QString &address, const QString &description, double amount) {
    ui->lineDescription->setText(description);
    ui->lineAmount->setText(QString::number(amount));
    ui->lineAddress->setText(address);
    ui->lineAddress->setCursorPosition(0);
}

void SendWidget::fillAddress(const QString &address) {
    ui->lineAddress->setText(address);
    ui->lineAddress->setCursorPosition(0);
}

void SendWidget::sendClicked() {
    double amount = 0.0;
    QString currency = ui->comboCurrencySelection->currentText();
    QString recipient = ui->lineAddress->text();  // @TODO: regex
    QString description = ui->lineDescription->text();
    if(recipient.isEmpty()) {
        QMessageBox::warning(this, "Malformed recipient", "The recipient address was not correct");
        return;
    }

    if (currency != "XMR") {
        amount = this->conversionAmount();
        if(amount <= 0.0) {
            QMessageBox::warning(this, "Fiat conversion error", "Could not create transaction.");
            return;
        }
        emit createTransaction(recipient, amount, description, false);
        return;
    }

    amount = this->amount();
    bool sendAll = amount == -1.0;
    if(amount == 0.0){
        QMessageBox::warning(this, "Amount error", "Invalid amount specified.");
        return;
    }

    emit createTransaction(recipient, amount, description, sendAll);
}

void SendWidget::aliasClicked() {
    auto address = ui->lineAddress->text();
    emit resolveOpenAlias(address);
}

void SendWidget::clearClicked() {
    ui->lineAddress->clear();
    ui->lineAmount->clear();
    ui->lineDescription->clear();
}

void SendWidget::btnMaxClicked() {
    ui->lineAmount->setText("all");
}

void SendWidget::updateConversionLabel() {
    auto amount = this->amount();
    if(amount == -1) return;
    ui->label_xmrAmount->setText("");
    if(amount <= 0) {
        ui->label_xmrAmount->hide();
        return;
    }

    QString currency = ui->comboCurrencySelection->currentText();
    if (currency != "XMR") {
        QString xmr_str = QString("%1 XMR").arg(QString::number(this->conversionAmount()));
        ui->label_xmrAmount->setText(xmr_str);
        ui->label_xmrAmount->show();
    }
}

double SendWidget::conversionAmount() {
    QString currency = ui->comboCurrencySelection->currentText();
    return AppContext::prices->convert(currency, "XMR", this->amount());
}

double SendWidget::amount() {
    // grab amount from "amount" text box
    QString amount = ui->lineAmount->text();
    if(amount == "all") return -1.0;
    amount.replace(',', '.');
    if(amount.isEmpty()) return 0.0;
    auto amount_num = amount.toDouble();
    if(amount_num <= 0) return 0.0;
    return amount_num;
}

void SendWidget::onOpenAliasResolved(const QString &address, const QString &openAlias) {
    this->fill(address, openAlias);
    ui->btn_openAlias->hide();
}

void SendWidget::onOpenAliasResolveError(const QString &msg) {
    QMessageBox::warning(this, "OpenAlias error", msg);
}

void SendWidget::clearFields() {
    ui->lineAddress->clear();
    ui->lineAmount->clear();
    ui->lineDescription->clear();
    ui->label_xmrAmount->clear();
}

void SendWidget::onWalletClosed() {
    this->clearFields();
}

void SendWidget::onInitiateTransaction() {
    ui->btnSend->setEnabled(false);
}

void SendWidget::onEndTransaction() {
    ui->btnSend->setEnabled(true);
}

SendWidget::~SendWidget() {
    delete ui;
}
