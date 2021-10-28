// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "VerifyProofDialog.h"
#include "ui_VerifyProofDialog.h"

#include <QMessageBox>

#include "libwalletqt/WalletManager.h"
#include "model/ModelUtils.h"
#include "utils/Utils.h"

VerifyProofDialog::VerifyProofDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::VerifyProofDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    m_success = QPixmap(":/assets/images/confirmed.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_failure = QPixmap(":/assets/images/expired.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->frame_status->hide();
    connect(ui->input_formattedProof, &QPlainTextEdit::textChanged, [this]{
       ui->frame_status->hide();
    });

    connect(ui->btn_verify, &QPushButton::clicked, this, &VerifyProofDialog::checkProof);
    connect(ui->btn_verifyFormattedProof, &QPushButton::clicked, this, &VerifyProofDialog::checkFormattedProof);

    connect(ui->btn_clear, &QPushButton::clicked, [this]{
        switch (ui->tabWidget->currentIndex()) {
            case 0:
                ui->lineEdit_spendTxID->clear();
                ui->input_SpendMessage->clear();
                ui->input_SpendProof->clear();
                break;
            case 1:
                ui->lineEdit_outTxID->clear();
                ui->lineEdit_outAddress->clear();
                ui->input_OutMessage->clear();
                ui->input_OutProof->clear();
                break;
            case 2:
                ui->lineEdit_inTxID->clear();
                ui->lineEdit_inAddress->clear();
                ui->input_inMessage->clear();
                ui->input_InProof->clear();
                break;
        }
    });

    connect(m_wallet, &Wallet::transactionProofVerified, this, &VerifyProofDialog::onTxProofVerified);
    connect(m_wallet, &Wallet::spendProofVerified, this, &VerifyProofDialog::onSpendProofVerified);

    ui->input_formattedProof->setFont(ModelUtils::getMonospaceFont());
}

void VerifyProofDialog::checkProof() {
    switch (ui->tabWidget->currentIndex()) {
        case 0:
            this->checkSpendProof(ui->lineEdit_spendTxID->text(), ui->input_SpendMessage->toPlainText(), ui->input_SpendProof->toPlainText());
            break;
        case 1:
            this->checkOutProof();
            break;
        case 2:
            this->checkInProof();
            break;
    }
}

void VerifyProofDialog::checkTxProof(const QString &txId, const QString &address, const QString &message,
                                     const QString &signature) {
    ui->btn_verifyFormattedProof->setEnabled(false);
    ui->btn_verify->setEnabled(false);
    m_wallet->checkTxProofAsync(txId, address, message, signature);
}

void VerifyProofDialog::checkSpendProof(const QString &txId, const QString &message, const QString &signature) {
    ui->btn_verifyFormattedProof->setEnabled(false);
    ui->btn_verify->setEnabled(false);
    m_wallet->checkSpendProofAsync(txId, message, signature);
}

void VerifyProofDialog::checkOutProof() {
    this->checkTxProof(ui->lineEdit_outTxID->text(), ui->lineEdit_outAddress->text(), ui->input_OutMessage->toPlainText(), ui->input_OutProof->toPlainText());
}

void VerifyProofDialog::checkInProof() {
    this->checkTxProof(ui->lineEdit_inTxID->text(), ui->lineEdit_inAddress->text(), ui->input_inMessage->toPlainText(), ui->input_InProof->toPlainText());
}

void VerifyProofDialog::checkFormattedProof() {
    QRegularExpression proof("-----BEGIN (?<type>\\w+)-----\\n"
                             "Network: (?<coin>\\w+) (?<network>\\w+)\\n"
                             "Txid: (?<txid>[0-9a-f]{64})\\n"
                             "(Address: (?<address>\\w+)\\n)?"
                             "\\n?"
                             "(?<message>.*?)\\n"
                             "-----BEGIN \\1 SIGNATURE-----\\n"
                             "\\n?"
                             "(?<signature>.*?)\\n"
                             "-----END \\1 SIGNATURE-----",
                             QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    QString formattedProof = ui->input_formattedProof->toPlainText();
    QRegularExpressionMatch match = proof.match(formattedProof);

    if (!match.hasMatch()) {
        this->proofStatus(false, "Unable to parse proof");
        return;
    }

    QString type = match.captured("type").toLower();
    QString coin = match.captured("coin").toLower();
    QString network = match.captured("network").toLower();
    QString txid = match.captured("txid");
    QString address = match.captured("address");
    QString message = match.captured("message");
    QString signature = match.captured("signature").remove('\n');

    QStringList validTypes = {"inproof", "spendproof", "outproof"};
    if (!validTypes.contains(type)) {
        this->proofStatus(false, QString("Unknown proof type: %1").arg(type));
        return;
    }

    if (coin != "monero") {
        this->proofStatus(false, QString("Can't verify proof for coin: %1").arg(coin));
        return;
    }

    QString walletNetwork = Utils::QtEnumToString(m_wallet->nettype()).toLower();
    if (network != walletNetwork) {
        this->proofStatus(false, QString("Can't verify proof for %1 network when %2 wallet is opened").arg(network, walletNetwork));
        return;
    }

    if (type == "outproof" || type == "inproof") {
        this->checkTxProof(txid, address, message, signature);
    }
    if (type == "spendproof") {
        this->checkSpendProof(txid, message, signature);
    }
}

void VerifyProofDialog::proofStatus(bool success, const QString &message) {
    if (ui->tabWidget_proofFormat->currentIndex() == 0) {
        ui->frame_status->show();
        ui->label_icon->setPixmap(success ? m_success : m_failure);
        ui->label_status->setText(message);
        ui->btn_verifyFormattedProof->setEnabled(true);
    }
    else {
        ui->btn_verify->setEnabled(true);
        success ? QMessageBox::information(this, "Information", message)
                : QMessageBox::warning(this, "Warning", message);
    }
}

void VerifyProofDialog::onTxProofVerified(TxProofResult r) {
    if (!r.success) {
        this->proofStatus(false, m_wallet->errorString());
        return;
    }

    if (!r.good) {
        this->proofStatus(false, "Proof is invalid");
        return;
    }

    this->proofStatus(true, QString("Proof is valid.\n\nThis address received %1 XMR, with %2 confirmation(s)").arg(WalletManager::displayAmount(r.received), QString::number(r.confirmations)));
}

void VerifyProofDialog::onSpendProofVerified(QPair<bool, bool> r) {
    if (!r.first) {
        this->proofStatus(false, m_wallet->errorString());
        return;
    }

    r.second ? this->proofStatus(true, "Proof is valid")
             : this->proofStatus(false, "Proof is invalid");
}

VerifyProofDialog::~VerifyProofDialog() = default;