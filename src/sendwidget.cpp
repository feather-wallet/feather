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
    m_ctx = MainWindow::getContext();

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
    ui->label_conversionAmount->setText("");
    ui->label_conversionAmount->hide();
    ui->btn_openAlias->hide();

    ui->label_PayTo->setHelpText("Recipient of the funds.\n\n"
                                 "You may enter a Monero address, or an alias (email-like address that forwards to a Monero address)");
    ui->label_Description->setHelpText("Description of the transaction (optional).\n\n"
                                       "The description is not sent to the recipient of the funds. It is stored in your wallet cache, "
                                       "and displayed in the 'History' tab.");
    ui->label_Amount->setHelpText("Amount to be sent.\n\nThis is the exact amount the recipient will receive. "
                                  "In addition to this amount a transaction fee will be subtracted from your balance. "
                                  "You will be able to review the transaction fee before the transaction is broadcast.\n\n"
                                  "To send all your balance, click the Max button to the right.");

    this->setupComboBox();
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

void SendWidget::fill(const QString &address, const QString &description, double amount) {
    ui->lineDescription->setText(description);
    ui->lineAddress->setText(address);
    ui->lineAddress->setCursorPosition(0);
    if (amount > 0)
        ui->lineAmount->setText(QString::number(amount));
    this->updateConversionLabel();
}

void SendWidget::fillAddress(const QString &address) {
    ui->lineAddress->setText(address);
    ui->lineAddress->setCursorPosition(0);
}

void SendWidget::sendClicked() {
    if (m_ctx->currentWallet->connectionStatus() != Wallet::ConnectionStatus_Connected) {
        QMessageBox::warning(this, "Error", "Unable to create transaction:\n\n"
                                            "Wallet is not connected to a node.\n"
                                            "Go to File -> Settings -> Node to manually connect to a node.");
        return;
    }

    double amount;
    QString currency = ui->comboCurrencySelection->currentText();
    QString recipient = ui->lineAddress->text().simplified().remove(' ');
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
    ui->label_conversionAmount->setText("");
    if(amount <= 0) {
        ui->label_conversionAmount->hide();
        return;
    }

    QString conversionAmountStr = [this]{
        QString currency = ui->comboCurrencySelection->currentText();
        if (currency != "XMR") {
            return QString("~%1 XMR").arg(QString::number(this->conversionAmount(), 'f'));

        } else {
            auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
            double conversionAmount = AppContext::prices->convert("XMR", preferredFiatCurrency, this->amount());
            return QString("~%1 %2").arg(QString::number(conversionAmount, 'f', 2), preferredFiatCurrency);
        };
    }();

    ui->label_conversionAmount->setText(conversionAmountStr);
    ui->label_conversionAmount->show();
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
    ui->label_conversionAmount->clear();
}

void SendWidget::onWalletClosed() {
    this->clearFields();
    ui->btnSend->setEnabled(true);
}

void SendWidget::onInitiateTransaction() {
    ui->btnSend->setEnabled(false);
}

void SendWidget::onEndTransaction() {
    ui->btnSend->setEnabled(true);
}

void SendWidget::setupComboBox() {
    ui->comboCurrencySelection->clear();

    QStringList defaultCurrencies = {"XMR", "USD", "EUR", "CNY", "JPY", "GBP"};
    QString preferredCurrency = config()->get(Config::preferredFiatCurrency).toString();

    if (defaultCurrencies.contains(preferredCurrency)) {
        defaultCurrencies.removeOne(preferredCurrency);
    }

    ui->comboCurrencySelection->insertItems(0, defaultCurrencies);
    ui->comboCurrencySelection->insertItem(1, preferredCurrency);
}

void SendWidget::onPreferredFiatCurrencyChanged() {
    this->updateConversionLabel();
    this->setupComboBox();
}

SendWidget::~SendWidget() {
    delete ui;
}
