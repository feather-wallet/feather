// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "transactioninfodialog.h"
#include "ui_transactioninfodialog.h"

#include "libwalletqt/CoinsInfo.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Transfer.h"
#include "utils.h"
#include "utils/ColorScheme.h"

#include <QMessageBox>

TransactionInfoDialog::TransactionInfoDialog(Wallet *wallet, TransactionInfo *txInfo, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TransactionInfoDialog)
        , m_wallet(wallet)
        , m_txInfo(txInfo)
{
    ui->setupUi(this);

    m_txid = txInfo->hash();
    ui->label_txid->setText(m_txid);

    QString txKey = m_wallet->getTxKey(txInfo->hash());
    if (txKey.isEmpty()) {
        ui->btn_CopyTxKey->setEnabled(false);
        ui->btn_CopyTxKey->setToolTip("Transaction key unknown");
    }
    m_txKey = txKey;

    connect(ui->btn_CopyTxKey, &QPushButton::pressed, this, &TransactionInfoDialog::copyTxKey);
    connect(ui->btn_createTxProof, &QPushButton::pressed, this, &TransactionInfoDialog::createTxProof);

    QString blockHeight = QString::number(txInfo->blockHeight());
    if (blockHeight == "0")
        blockHeight = "Unconfirmed";

    ui->label_status->setText(QString("Status: %1 confirmations").arg(txInfo->confirmations()));
    ui->label_date->setText(QString("Date: %1").arg(txInfo->timestamp().toString("yyyy-MM-dd HH:mm")));
    ui->label_blockHeight->setText(QString("Block height: %1").arg(blockHeight));

    QString direction = txInfo->direction() == TransactionInfo::Direction_In ? "received" : "sent";
    ui->label_amount->setText(QString("Amount %1: %2").arg(direction, txInfo->displayAmount()));

    QString fee = txInfo->fee().isEmpty() ? "n/a" : txInfo->fee();
    ui->label_fee->setText(QString("Fee: %1").arg(txInfo->isCoinbase() ? WalletManager::displayAmount(0) : fee));
    ui->label_unlockTime->setText(QString("Unlock time: %1 (height)").arg(txInfo->unlockTime()));

    qDebug() << m_wallet->coins()->coins_from_txid(txInfo->hash());

    QTextCursor cursor = ui->destinations->textCursor();
    for (const auto& transfer : txInfo->transfers()) {
        auto address = transfer->address();
        auto amount = WalletManager::displayAmount(transfer->amount());
        auto index = m_wallet->subaddressIndex(address);
        cursor.insertText(address, Utils::addressTextFormat(index));
        cursor.insertText(QString(" %1").arg(amount), QTextCharFormat());
        cursor.insertBlock();
    }
    if (txInfo->transfers().size() == 0) {
        ui->frameDestinations->hide();
    }

    m_txProofDialog = new TxProofDialog(this, m_wallet, txInfo);

    this->adjustSize();
}

void TransactionInfoDialog::copyTxKey() {
    Utils::copyToClipboard(m_txKey);
}

void TransactionInfoDialog::createTxProof() {
    m_txProofDialog->show();
}

TransactionInfoDialog::~TransactionInfoDialog() {
    delete ui;
}
