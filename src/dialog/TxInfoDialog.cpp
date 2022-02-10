// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "TxInfoDialog.h"
#include "ui_TxInfoDialog.h"

#include <QMessageBox>
#include <QScrollBar>

#include "appcontext.h"
#include "config.h"
#include "constants.h"
#include "libwalletqt/Coins.h"
#include "libwalletqt/CoinsInfo.h"
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/Transfer.h"
#include "libwalletqt/WalletManager.h"
#include "model/ModelUtils.h"
#include "Utils.h"
#include "utils/Icons.h"

TxInfoDialog::TxInfoDialog(QSharedPointer<AppContext> ctx, TransactionInfo *txInfo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TxInfoDialog)
    , m_ctx(std::move(ctx))
    , m_txInfo(txInfo)
    , m_txProofDialog(new TxProofDialog(this, m_ctx, txInfo))
{
    ui->setupUi(this);

    ui->btn_viewOnBlockExplorer->setIcon(icons()->icon("external-link.svg"));
    ui->btn_viewOnBlockExplorer->setToolTip("View on block explorer");
    connect(ui->btn_viewOnBlockExplorer, &QPushButton::clicked, this, &TxInfoDialog::viewOnBlockExplorer);

    m_txid = txInfo->hash();
    ui->label_txid->setText(m_txid);

    connect(ui->btn_copyTxID, &QPushButton::clicked, this, &TxInfoDialog::copyTxID);
    connect(ui->btn_CopyTxKey, &QPushButton::clicked, this, &TxInfoDialog::copyTxKey);
    connect(ui->btn_createTxProof, &QPushButton::clicked, this, &TxInfoDialog::createTxProof);

    connect(m_ctx->wallet, &Wallet::newBlock, this, &TxInfoDialog::updateData);

    this->setData(txInfo);

    if ((txInfo->isFailed() || txInfo->isPending()) && txInfo->direction() != TransactionInfo::Direction_In) {
        connect(ui->btn_rebroadcastTx, &QPushButton::pressed, [this]{
            emit resendTranscation(m_txid);
        });
    } else {
        ui->btn_rebroadcastTx->hide();
    }

    if (txInfo->direction() == TransactionInfo::Direction_In) {
        ui->btn_CopyTxKey->setDisabled(true);
        ui->btn_CopyTxKey->setToolTip("No tx secret key available for incoming transactions.");
    }

//
//    if (txInfo->direction() == TransactionInfo::Direction_Out) {
//        // TODO: this will not properly represent coinjoin-like transactions.
//        QVector<CoinsInfo*> coins = m_ctx->wallet->coins()->coins_from_txid(m_txid);
//        QTextCursor c_i = ui->inputs->textCursor();
//        QString inputs_str;
//        for (const auto &coin : coins) {
//            inputs_str += QString("%1 %2\n").arg(coin->pubKey(), coin->displayAmount());
//        }
//        ui->inputs->setText(inputs_str);
//        ui->label_inputs->setText(QString("Inputs (%1)").arg(QString::number(coins.size())));
//        this->adjustHeight(ui->inputs, coins.size());
//    } else {
    ui->frameInputs->hide();
//    }

    QTextCursor cursor = ui->outputs->textCursor();

    auto transfers = txInfo->transfers();
    if (!transfers.isEmpty()) {
        for (const auto& transfer : transfers) {
            auto address = transfer->address();
            auto amount = WalletManager::displayAmount(transfer->amount());
            auto index = m_ctx->wallet->subaddressIndex(address);
            cursor.insertText(address, Utils::addressTextFormat(index, transfer->amount()));
            cursor.insertText(QString(" %1").arg(amount), QTextCharFormat());
            cursor.insertBlock();
        }
        ui->label_outputs->setText(QString("Destinations (%2)").arg(QString::number(transfers.size())));
        this->adjustHeight(ui->outputs, transfers.size());
    } else {
        ui->frameOutputs->hide();
    }

    this->adjustSize();

    // Don't autofocus any of the buttons. There is probably a better way for this.
    ui->label_txid->setFocus();
}

void TxInfoDialog::adjustHeight(QTextEdit *textEdit, qreal docHeight) {
    QCoreApplication::processEvents();

    qreal lineHeight = QFontMetrics(ui->outputs->document()->defaultFont()).height();

    int h = int(docHeight * (lineHeight + 2) + 11);
    h = qMin(qMax(h, 100), 600);
    textEdit->setMinimumHeight(h);
    textEdit->setMaximumHeight(h);
    textEdit->verticalScrollBar()->hide();
}

void TxInfoDialog::setData(TransactionInfo *tx) {
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

void TxInfoDialog::updateData() {
    TransactionInfo *tx = m_ctx->wallet->history()->transaction(m_txid);
    if (!tx) return;
    this->setData(tx);
}

void TxInfoDialog::copyTxID() {
    Utils::copyToClipboard(m_txid);
}

void TxInfoDialog::copyTxKey() {
    m_ctx->wallet->getTxKeyAsync(m_txid, [this](QVariantMap map){
        QString txKey = map.value("tx_key").toString();
        if (txKey.isEmpty()) {
            QMessageBox::warning(this, "Unable to copy transaction key", "Transaction key unknown");
        } else {
            Utils::copyToClipboard(txKey);
            QMessageBox::information(this, "Transaction key copied", "Transaction key copied to clipboard.");
        }
    });
}

void TxInfoDialog::createTxProof() {
    m_txProofDialog->show();
    m_txProofDialog->getTxKey();
}

void TxInfoDialog::viewOnBlockExplorer() {
    Utils::externalLinkWarning(this, Utils::blockExplorerLink(config()->get(Config::blockExplorer).toString(), constants::networkType, m_txid));
}

TxInfoDialog::~TxInfoDialog() = default;