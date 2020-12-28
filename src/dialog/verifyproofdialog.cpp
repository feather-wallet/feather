// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "verifyproofdialog.h"
#include "ui_verifyproofdialog.h"

#include "libwalletqt/WalletManager.h"

#include <QMessageBox>

VerifyProofDialog::VerifyProofDialog(Wallet *wallet, QWidget *parent)
        : QDialog(parent)
        , m_wallet(wallet)
        , ui(new Ui::VerifyProofDialog)
{
    ui->setupUi(this);

    connect(ui->btn_verify, &QPushButton::clicked, this, &VerifyProofDialog::checkProof);

    connect(ui->btn_clear, &QPushButton::clicked, [this]{
        switch (ui->tabWidget->currentIndex()) {
            case 0:
                ui->lineEdit_spendTxID->clear();
                ui->lineEdit_spendMessage->clear();
                ui->input_SpendProof->clear();
                break;
            case 1:
                ui->lineEdit_outTxID->clear();
                ui->lineEdit_outAddress->clear();
                ui->lineEdit_outMessage->clear();
                ui->input_OutProof->clear();
                break;
            case 2:
                ui->lineEdit_inTxID->clear();
                ui->lineEdit_inAddress->clear();
                ui->lineEdit_inMessage->clear();
                ui->input_InProof->clear();
                break;
        }
    });
}

VerifyProofDialog::~VerifyProofDialog()
{
    delete ui;
}

void VerifyProofDialog::checkProof() {
    switch (ui->tabWidget->currentIndex()) {
        case 0:
            this->checkSpendProof();
            break;
        case 1:
            this->checkOutProof();
            break;
        case 2:
            this->checkInProof();
            break;
    }
}

void VerifyProofDialog::checkSpendProof() {
    auto r = m_wallet->checkSpendProof(ui->lineEdit_spendTxID->text(), ui->lineEdit_spendMessage->text(), ui->input_SpendProof->toPlainText());

    if (!r.first) {
        QMessageBox::information(this, "Information", m_wallet->errorString());
        return;
    }

    r.second ? QMessageBox::information(this, "Information", "Proof is valid")
             : QMessageBox::warning(this, "Warning", "Proof is invalid");
}

void VerifyProofDialog::checkOutProof() {
    this->checkTxProof(ui->lineEdit_outTxID->text(), ui->lineEdit_outAddress->text(), ui->lineEdit_outMessage->text(), ui->input_OutProof->toPlainText());
}

void VerifyProofDialog::checkInProof() {
    this->checkTxProof(ui->lineEdit_inTxID->text(), ui->lineEdit_inAddress->text(), ui->lineEdit_inMessage->text(), ui->input_InProof->toPlainText());
}

void VerifyProofDialog::checkTxProof(const QString &txId, const QString &address, const QString &message,
                                     const QString &signature) {
    TxProofResult r = m_wallet->checkTxProof(txId, address, message, signature);

    if (!r.success) {
        QMessageBox::information(this, "Information", m_wallet->errorString());
        return;
    }

    if (!r.good) {
        QMessageBox::warning(this, "Warning", "Proof is invalid");
        return;
    }

    QString msg = QString("This address received %1 monero, with %2 confirmation(s)").arg(WalletManager::displayAmount(r.received), QString::number(r.confirmations));
    QMessageBox::information(this, "Information", QString("Proof is valid.\n\n%1").arg(msg));
}