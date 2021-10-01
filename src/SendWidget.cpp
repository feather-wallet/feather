// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "SendWidget.h"
#include "ui_SendWidget.h"

#include <QMessageBox>

#include "ColorScheme.h"
#include "constants.h"
#include "utils/AppData.h"
#include "Icons.h"

#ifdef WITH_SCANNER
#include "qrcode_scanner/QrCodeScanDialog.h"
#include <QtMultimedia/QCameraInfo>
#endif

SendWidget::SendWidget(QSharedPointer<AppContext> ctx, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SendWidget)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    QString amount_rx = R"(^\d{0,8}[\.,]\d{0,12}|(all)$)";
    QRegExp rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineAmount->setValidator(validator);

    connect(m_ctx.get(), &AppContext::initiateTransaction, this, &SendWidget::onInitiateTransaction);
    connect(m_ctx.get(), &AppContext::endTransaction, this, &SendWidget::onEndTransaction);
    connect(m_ctx.get(), &AppContext::openAliasResolved, this, &SendWidget::onOpenAliasResolved);
    connect(m_ctx.get(), &AppContext::openAliasResolveError, this, &SendWidget::onOpenAliasResolveError);

    connect(ui->btnScan, &QPushButton::clicked, this, &SendWidget::scanClicked);
    connect(ui->btnSend, &QPushButton::clicked, this, &SendWidget::sendClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &SendWidget::clearClicked);
    connect(ui->btnMax, &QPushButton::clicked, this, &SendWidget::btnMaxClicked);
    connect(ui->comboCurrencySelection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SendWidget::currencyComboChanged);
    connect(ui->lineAmount, &QLineEdit::textChanged, this, &SendWidget::amountEdited);
    connect(ui->lineAddress, &QPlainTextEdit::textChanged, this, &SendWidget::addressEdited);
    connect(ui->btn_openAlias, &QPushButton::clicked, this, &SendWidget::aliasClicked);
    connect(ui->lineAddress, &PayToEdit::dataPasted, this, &SendWidget::onDataPasted);
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

    ui->lineAddress->setNetType(constants::networkType);
    this->setupComboBox();
}

void SendWidget::currencyComboChanged(int index) {
    QString amount = ui->lineAmount->text();
    if(amount.isEmpty()) return;
    this->amountEdited(amount);
}

void SendWidget::addressEdited() {
    QVector<PartialTxOutput> outputs = ui->lineAddress->getOutputs();

    bool freezeAmounts = !outputs.empty();

    ui->lineAmount->setReadOnly(freezeAmounts);
    ui->lineAmount->setFrame(!freezeAmounts);
    ui->btnMax->setDisabled(freezeAmounts);
    ui->comboCurrencySelection->setDisabled(freezeAmounts);

    if (!outputs.empty()) {
        ui->lineAmount->setText(WalletManager::displayAmount(ui->lineAddress->getTotal(), false));
        ui->comboCurrencySelection->setCurrentIndex(0);
    }

    ui->btn_openAlias->setVisible(ui->lineAddress->isOpenAlias());
}

void SendWidget::amountEdited(const QString &text) {
    this->updateConversionLabel();
}

void SendWidget::fill(double amount) {
    ui->lineAmount->setText(QString::number(amount));
}

void SendWidget::fill(const QString &address, const QString &description, double amount) {
    ui->lineAddress->setText(address);
    ui->lineAddress->moveCursor(QTextCursor::Start);

    ui->lineDescription->setText(description);

    if (amount > 0)
        ui->lineAmount->setText(QString::number(amount));
    ui->lineAmount->setFocus();

    this->updateConversionLabel();
}

void SendWidget::fillAddress(const QString &address) {
    ui->lineAddress->setText(address);
    ui->lineAddress->moveCursor(QTextCursor::Start);
}

void SendWidget::scanClicked() {
#ifdef WITH_SCANNER
    auto cameras = QCameraInfo::availableCameras();
    if (cameras.count() < 1) {
        QMessageBox::warning(this, "QR code scanner", "No available cameras found.");
        return;
    }

    auto *dialog = new QrCodeScanDialog(this);
    dialog->exec();
    ui->lineAddress->setText(dialog->decodedString);
    dialog->deleteLater();
#else
    QMessageBox::warning(this, "QR scanner", "Feather was built without webcam QR scanner support.");
#endif
}

void SendWidget::sendClicked() {
    if (!m_ctx->wallet->isConnected()) {
        QMessageBox::warning(this, "Error", "Unable to create transaction:\n\n"
                                            "Wallet is not connected to a node.\n"
                                            "Go to File -> Settings -> Node to manually connect to a node.");
        return;
    }

    QString currency = ui->comboCurrencySelection->currentText();
    QString recipient = ui->lineAddress->text().simplified().remove(' ');
    QString description = ui->lineDescription->text();
    if(recipient.isEmpty()) {
        QMessageBox::warning(this, "Malformed recipient", "The recipient address was not correct");
        return;
    }

    QVector<PartialTxOutput> outputs = ui->lineAddress->getOutputs();
    QVector<PayToLineError> errors = ui->lineAddress->getErrors();
    if (errors.size() > 0 && ui->lineAddress->isMultiline()) {
        QString errorText;
        for (auto &error: errors) {
            errorText += QString("Line #%1:\n%2\n").arg(QString::number(error.idx + 1), error.error);
        }

        QMessageBox::warning(this, "Warning", QString("Invalid lines found:\n\n%1").arg(errorText));
        return;
    }

    if (outputs.size() > 0) { // multi destination transaction
        if (outputs.size() > 16) {
            QMessageBox::warning(this, "Warning", "Maximum number of outputs (16) exceeded.");
            return;
        }

        QVector<QString> addresses;
        QVector<quint64> amounts;
        for (auto &output : outputs) {
            addresses.push_back(output.address);
            amounts.push_back(output.amount);
        }

        m_ctx->onCreateTransactionMultiDest(addresses, amounts, description);
        return;
    }

    quint64 amount;
    if (currency == "XMR") {
        amount = this->amount();
        bool sendAll = (ui->lineAmount->text() == "all");
        if (amount == 0 && !sendAll) {
            QMessageBox::warning(this, "Amount error", "Invalid amount specified.");
            return;
        }
        m_ctx->onCreateTransaction(recipient, amount, description, sendAll);
    } else {
        amount = WalletManager::amountFromDouble(this->conversionAmount());
        if (amount == 0) {
            QMessageBox::warning(this, "Fiat conversion error", "Could not create transaction.");
            return;
        }
        m_ctx->onCreateTransaction(recipient, amount, description, false);
    }
}

void SendWidget::aliasClicked() {
    auto address = ui->lineAddress->text();
    m_ctx->onOpenAliasResolve(address);
}

void SendWidget::clearClicked() {
    ui->lineAddress->clear();
    ui->lineAmount->clear();
    ui->lineDescription->clear();
}

void SendWidget::btnMaxClicked() {
    ui->lineAmount->setText("all");
    this->updateConversionLabel();
}

void SendWidget::updateConversionLabel() {
    auto amount = this->amountDouble();

    ui->label_conversionAmount->setText("");
    if (amount <= 0) {
        ui->label_conversionAmount->hide();
        return;
    }

    QString conversionAmountStr = [this]{
        QString currency = ui->comboCurrencySelection->currentText();
        if (currency != "XMR") {
            return QString("~%1 XMR").arg(QString::number(this->conversionAmount(), 'f'));

        } else {
            auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
            double conversionAmount = appData()->prices.convert("XMR", preferredFiatCurrency, this->amountDouble());
            return QString("~%1 %2").arg(QString::number(conversionAmount, 'f', 2), preferredFiatCurrency);
        }
    }();

    ui->label_conversionAmount->setText(conversionAmountStr);
    ui->label_conversionAmount->show();
}

double SendWidget::conversionAmount() {
    QString currency = ui->comboCurrencySelection->currentText();
    return appData()->prices.convert(currency, "XMR", this->amountDouble());
}

quint64 SendWidget::amount() {
    // grab amount from "amount" text box
    QString amount = ui->lineAmount->text();
    if (amount == "all") return 0;

    amount.replace(',', '.');
    if (amount.isEmpty()) return 0;

    return WalletManager::amountFromString(amount);
}

double SendWidget::amountDouble() {
    quint64 amount = this->amount();
    return amount / constants::cdiv;
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

void SendWidget::payToMany() {
    ui->lineAddress->payToMany();
}

void SendWidget::onInitiateTransaction() {
    ui->btnSend->setEnabled(false);
}

void SendWidget::onEndTransaction() {
    ui->btnSend->setEnabled(true);
}

void SendWidget::onDataPasted(const QString &data) {
    if (!data.isEmpty()) {
        QVariantMap uriData = m_ctx->wallet->parse_uri_to_object(data);
        if (!uriData.contains("error")) {
            ui->lineAddress->setText(uriData.value("address").toString());
            ui->lineDescription->setText(uriData.value("tx_description").toString());
            ui->lineAmount->setText(uriData.value("amount").toString());
        } else {
            ui->lineAddress->setText(data);
        }
    }
    else {
        QMessageBox::warning(this, "Error", "No Qr Code found.");
    }
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

void SendWidget::skinChanged() {
    if (ColorScheme::hasDarkBackground(this)) {
        ui->btnScan->setIcon(icons()->icon("camera_white.png"));
    } else {
        ui->btnScan->setIcon(icons()->icon("camera_dark.png"));
    }
}

SendWidget::~SendWidget() = default;