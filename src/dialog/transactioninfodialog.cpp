// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "transactioninfodialog.h"
#include "ui_transactioninfodialog.h"

#include "libwalletqt/CoinsInfo.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Transfer.h"
#include "libwalletqt/TransactionHistory.h"
#include "utils.h"
#include "utils/ColorScheme.h"
#include "model/ModelUtils.h"
#include "config.h"
#include "appcontext.h"

#include <QMessageBox>
#include <QScrollBar>

TransactionInfoDialog::TransactionInfoDialog(QSharedPointer<AppContext> ctx, TransactionInfo *txInfo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TransactionInfoDialog)
    , m_ctx(std::move(ctx))
    , m_txInfo(txInfo)
{
    ui->setupUi(this);

    m_txid = txInfo->hash();
    ui->label_txid->setText(m_txid);

    m_txKey = m_ctx->wallet->getTxKey(txInfo->hash());
    if (m_txKey.isEmpty()) {
        ui->btn_CopyTxKey->setEnabled(false);
        ui->btn_CopyTxKey->setToolTip("Transaction key unknown");
    }

    connect(ui->btn_CopyTxKey, &QPushButton::pressed, this, &TransactionInfoDialog::copyTxKey);
    connect(ui->btn_createTxProof, &QPushButton::pressed, this, &TransactionInfoDialog::createTxProof);

    connect(m_ctx->wallet.get(), &Wallet::newBlock, this, &TransactionInfoDialog::updateData);

    this->setData(txInfo);

    if (m_ctx->txCache.contains(txInfo->hash()) && (txInfo->isFailed() || txInfo->isPending()) && txInfo->direction() != TransactionInfo::Direction_In) {
        connect(ui->btn_rebroadcastTx, &QPushButton::pressed, [this]{
            emit resendTranscation(m_txid);
        });
    } else {
        ui->btn_rebroadcastTx->hide();
    }

    QTextCursor cursor = ui->destinations->textCursor();
    for (const auto& transfer : txInfo->transfers()) {
        auto address = transfer->address();
        auto amount = WalletManager::displayAmount(transfer->amount());
        auto index = m_ctx->wallet->subaddressIndex(address);
        cursor.insertText(address, Utils::addressTextFormat(index));
        cursor.insertText(QString(" %1").arg(amount), QTextCharFormat());
        cursor.insertBlock();
    }

    if (txInfo->transfers().size() == 0) {
        ui->frameDestinations->hide();
    }

    m_txProofDialog = new TxProofDialog(this, m_ctx, txInfo);

    QCoreApplication::processEvents();

    qreal lineHeight = QFontMetrics(ui->destinations->document()->defaultFont()).height();
    qreal docHeight = txInfo->transfers().size();
    int h = int(docHeight * (lineHeight + 2) + 11);
    h = qMin(qMax(h, 100), 600);
    ui->destinations->setMinimumHeight(h);
    ui->destinations->setMaximumHeight(h);
    ui->destinations->verticalScrollBar()->hide();

    this->adjustSize();
}

void TransactionInfoDialog::setData(TransactionInfo* tx) {
    QString blockHeight = QString::number(tx->blockHeight());

    if (tx->isFailed()) {
        ui->label_status->setText("Status: Failed (node was unable to relay transaction)");
    }
    if (blockHeight == "0") {
        ui->label_status->setText("Status: Unconfirmed (in mempool)");
    }
    else {
        QString dateTimeFormat = QString("%1 %2").arg(config()->get(Config::dateFormat).toString(), config()->get(Config::timeFormat).toString());
        QString date = tx->timestamp().toString(dateTimeFormat);
        QString statusText = QString("Status: Included in block %1 (%2 confirmations) on %3").arg(blockHeight, QString::number(tx->confirmations()), date);
        ui->label_status->setText(statusText);
    }


    if (tx->confirmationsRequired() > tx->confirmations()) {
        bool mandatoryLock = tx->confirmationsRequired() == 10;
        QString confsRequired = QString::number(tx->confirmationsRequired() - tx->confirmations());
        ui->label_lock->setText(QString("Lock: Outputs become spendable in %1 blocks (%2)").arg(confsRequired, mandatoryLock ? "consensus rule" : "specified by sender"));
    } else {
        ui->label_lock->setText("Lock: Outputs are spendable");
    }

    QString direction = tx->direction() == TransactionInfo::Direction_In ? "received" : "sent";
    ui->label_amount->setText(QString("Amount %1: %2 XMR").arg(direction, tx->displayAmount()));

    QString fee;
    if (tx->isCoinbase())
        fee = "Not applicable";
    else if (tx->direction() == TransactionInfo::Direction_In)
        fee = "Paid by sender";
    else if (tx->fee().isEmpty())
        fee = "N/A";
    else
        fee = QString("%1 XMR").arg(tx->fee());

    ui->label_fee->setText(QString("Fee: %1").arg(fee));

}

void TransactionInfoDialog::updateData() {
    TransactionInfo* tx = m_ctx->wallet->history()->transaction(m_txid);
    if (!tx) return;
    this->setData(tx);
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
