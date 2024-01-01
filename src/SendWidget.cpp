// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "SendWidget.h"
#include "ui_SendWidget.h"

#include <QMessageBox>

#include "ColorScheme.h"
#include "constants.h"
#include "utils/AppData.h"
#include "utils/config.h"
#include "Icons.h"
#include "libwalletqt/WalletManager.h"

#if defined(WITH_SCANNER)
#include "wizard/offline_tx_signing/OfflineTxSigningWizard.h"
#include "qrcode/scanner/QrCodeScanDialog.h"
#include <QMediaDevices>
#endif

SendWidget::SendWidget(Wallet *wallet, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SendWidget)
    , m_wallet(wallet)
{
    ui->setupUi(this);

    QString amount_rx = R"(^\d{0,8}[\.,]\d{0,12}|(all)$)";
    QRegularExpression rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->lineAmount->setValidator(validator);

    connect(m_wallet, &Wallet::initiateTransaction, this, &SendWidget::disableSendButton);
    connect(m_wallet, &Wallet::transactionCreated, this, &SendWidget::enableSendButton);
    connect(m_wallet, &Wallet::beginCommitTransaction, this, &SendWidget::disableSendButton);
    connect(m_wallet, &Wallet::transactionCommitted, this, &SendWidget::enableSendButton);

    connect(WalletManager::instance(), &WalletManager::openAliasResolved, this, &SendWidget::onOpenAliasResolved);

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

    ui->label_PayTo->setHelpText("Recipient of the funds",
                                 "You may enter a Monero address, or an alias (email-like address that forwards to a Monero address)",
                                 "send_transaction");
    ui->label_Description->setHelpText("Description of the transaction (optional)",
                                       "The description is not sent to the recipient of the funds. It is stored in your wallet cache, "
                                       "and displayed in the 'History' tab.",
                                       "send_transaction");
    ui->label_Amount->setHelpText("Amount to be sent","This is the exact amount the recipient will receive. "
                                  "In addition to this amount a transaction fee will be subtracted from your balance. "
                                  "You will be able to review the transaction fee before the transaction is broadcast.\n\n"
                                  "To send all your balance, click the Max button to the right.","send_transaction");

    ui->lineAddress->setNetType(constants::networkType);
    this->setupComboBox();
}

void SendWidget::currencyComboChanged(int index) {
    Q_UNUSED(index)
    QString amount = ui->lineAmount->text();
    if (amount.isEmpty()) {
        return;
    }
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

    // Clear donation description if address no longer matches
    if (ui->lineDescription->text() == constants::donationDescription && ui->lineAddress->text() != constants::donationAddress) {
        ui->lineDescription->clear();
    }
}

void SendWidget::amountEdited(const QString &text) {
    Q_UNUSED(text)
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
#if defined(WITH_SCANNER)
    auto cameras = QMediaDevices::videoInputs();
    if (cameras.empty()) {
        Utils::showError(this, "Can't open QR scanner", "No available cameras found");
        return;
    }

    auto dialog = new QrCodeScanDialog(this, false);
    dialog->exec();
    ui->lineAddress->setText(dialog->decodedString());
    dialog->deleteLater();
#else
    Utils::showError(this, "Can't open QR scanner", "Feather was built without webcam QR scanner support");
#endif
}

void SendWidget::sendClicked() {
    if (!m_wallet->isConnected()) {
        Utils::showError(this, "Unable to create transaction", "Wallet is not connected to a node.",
                         {"Wait for the wallet to automatically connect to a node.", "Go to File -> Settings -> Network -> Node to manually connect to a node."},
                         "nodes");
        return;
    }

    if (!m_wallet->isSynchronized()) {
        Utils::showError(this, "Unable to create transaction", "Wallet is not synchronized", {"Wait for wallet synchronization to complete"}, "synchronization");
        return;
    }

    QString recipient = ui->lineAddress->text().simplified().remove(' ');
    if (recipient.isEmpty()) {
        Utils::showError(this, "Unable to create transaction", "No address was entered", {"Enter an address in the 'Pay to' field."}, "send_transaction");
        return;
    }

    QVector<PartialTxOutput> outputs = ui->lineAddress->getOutputs();
    QVector<PayToLineError> errors = ui->lineAddress->getErrors();
    if (!errors.empty() && ui->lineAddress->isMultiline()) {
        QString errorText;
        for (auto &error: errors) {
            errorText += QString("Line #%1:\n%2\n").arg(QString::number(error.idx + 1), error.error);
        }

        Utils::showError(this, "Unable to create transaction", QString("Invalid address lines found:\n\n%1").arg(errorText), {}, "pay_to_many");
        return;
    }

    QString description = ui->lineDescription->text();

    if (!outputs.empty()) { // multi destination transaction
        if (outputs.size() > 16) {
            Utils::showError(this, "Unable to create transaction", "Maximum number of outputs (16) exceeded.", {}, "pay_to_many");
            return;
        }

        QVector<QString> addresses;
        QVector<quint64> amounts;
        for (auto &output : outputs) {
            addresses.push_back(output.address);
            amounts.push_back(output.amount);
        }

        m_wallet->createTransactionMultiDest(addresses, amounts, description);
        return;
    }

    bool sendAll = (ui->lineAmount->text() == "all");
    QString currency = ui->comboCurrencySelection->currentText();
    quint64 amount = this->amount();

    if (amount == 0 && !sendAll) {
        Utils::showError(this, "Unable to create transaction", "No amount was entered", {}, "send_transaction", "Amount field");
        return;
    }

    if (currency != "XMR" && !sendAll) {
        // Convert fiat amount to XMR, but only if we're not sending the entire balance
        amount = WalletManager::amountFromDouble(this->conversionAmount());
    }

    quint64 unlocked_balance = m_wallet->unlockedBalance();
    quint64 total_balance = m_wallet->balance();
    if (total_balance == 0) {
        Utils::showError(this, "Unable to create transaction", "No money to spend");
        return;
    }

    if (unlocked_balance == 0) {
        Utils::showError(this, "Unable to create transaction", QString("No spendable balance.\n\n%1 XMR becomes spendable within 10 blocks (~20 minutes).").arg(WalletManager::displayAmount(total_balance - unlocked_balance)), {"Wait for more balance to unlock.", "Click 'Help' to learn more about how balance works."}, "balance");
        return;
    }

    if (!sendAll && amount > unlocked_balance) {
        Utils::showError(this, "Unable to create transaction", QString("Not enough money to spend.\n\n"
                                                                       "Spendable balance: %1").arg(WalletManager::displayAmount(unlocked_balance)));
        return;
    }

    // TODO: allow using file-only airgapped signing without scanner

    if (m_wallet->keyImageSyncNeeded(amount, sendAll)) {
        #if defined(WITH_SCANNER)
        OfflineTxSigningWizard wizard(this, m_wallet);
        auto r = wizard.exec();
        m_wallet->setForceKeyImageSync(false);

        if (r == QDialog::Rejected) {
            return;
        }
        #else
        Utils::showError(this, "Can't open offline transaction signing wizard", "Feather was built without webcam QR scanner support");
        return;
        #endif
    }

    m_wallet->createTransaction(recipient, amount, description, sendAll);
}

void SendWidget::aliasClicked() {
    ui->btn_openAlias->setEnabled(false);
    auto alias = ui->lineAddress->text();
    WalletManager::instance()->resolveOpenAliasAsync(alias);
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

    if (conf()->get(Config::disableWebsocket).toBool()) {
        return;
    }

    QString conversionAmountStr = [this]{
        QString currency = ui->comboCurrencySelection->currentText();
        if (currency != "XMR") {
            return QString("~%1 XMR").arg(QString::number(this->conversionAmount(), 'f'));

        } else {
            auto preferredFiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
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
    if (amount == "all") {
        return 0;
    }

    amount.replace(',', '.');
    if (amount.isEmpty()) {
        return 0;
    }

    return WalletManager::amountFromString(amount);
}

double SendWidget::amountDouble() {
    quint64 amount = this->amount();
    return amount / constants::cdiv;
}

void SendWidget::onOpenAliasResolved(const QString &openAlias, const QString &address, bool dnssecValid) {
    ui->btn_openAlias->setEnabled(true);

    if (address.isEmpty()) {
        Utils::showError(this, "Unable to resolve OpenAlias", "Address empty.");
        return;
    }

    if (!dnssecValid) {
        Utils::showError(this, "Unable to resolve OpenAlias", "Address found, but the DNSSEC signatures could not be verified, so this address may be spoofed.");
        return;
    }

    bool valid = WalletManager::addressValid(address, constants::networkType);
    if (!valid) {
        Utils::showError(this, "Unable to resolve OpenAlias", QString("Address validation failed.\n\nOpenAlias: %1\nAddress: %2").arg(openAlias, address));
        return;
    }

    this->fill(address, openAlias);
    ui->btn_openAlias->hide();
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

void SendWidget::disableSendButton() {
    ui->btnSend->setEnabled(false);
}

void SendWidget::enableSendButton() {
    if (m_disallowSending) {
        return;
    }
    ui->btnSend->setEnabled(true);
}

void SendWidget::disallowSending() {
    m_disallowSending = true;
    ui->btnSend->setEnabled(false);
}

void SendWidget::setWebsocketEnabled(bool enabled) {
    this->updateConversionLabel();
    if (enabled) {
        this->setupComboBox();
    } else {
        ui->comboCurrencySelection->clear();
        ui->comboCurrencySelection->insertItem(0, "XMR");
    }
}

void SendWidget::onDataPasted(const QString &data) {
    if (!data.isEmpty()) {
        QVariantMap uriData = m_wallet->parse_uri_to_object(data);
        if (!uriData.contains("error")) {
            ui->lineAddress->setText(uriData.value("address").toString());
            ui->lineDescription->setText(uriData.value("tx_description").toString());
            ui->lineAmount->setText(uriData.value("amount").toString());
        } else {
            ui->lineAddress->setText(data);
        }
    }
    else {
        Utils::showError(this, "Unable to decode QR code", "No QR code found.");
    }
}

void SendWidget::setupComboBox() {
    ui->comboCurrencySelection->clear();

    QStringList defaultCurrencies = {"XMR", "USD", "EUR", "CNY", "JPY", "GBP"};
    QString preferredCurrency = conf()->get(Config::preferredFiatCurrency).toString();

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