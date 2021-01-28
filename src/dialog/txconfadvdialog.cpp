// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "txconfadvdialog.h"
#include "ui_txconfadvdialog.h"
#include "qrcode/QrCode.h"
#include "dialog/qrcodedialog.h"
#include "libwalletqt/Transfer.h"
#include "libwalletqt/Input.h"
#include "model/ModelUtils.h"
#include "utils/ColorScheme.h"

#include <QFileDialog>
#include <QMessageBox>

TxConfAdvDialog::TxConfAdvDialog(AppContext *ctx, const QString &description, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TxConfAdvDialog)
        , m_ctx(ctx)
        , m_exportUnsignedMenu(new QMenu(this))
        , m_exportSignedMenu(new QMenu(this))
{
    ui->setupUi(this);

    m_exportUnsignedMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::unsignedCopy);
    m_exportUnsignedMenu->addAction("Show as QR code", this, &TxConfAdvDialog::unsignedQrCode);
    m_exportUnsignedMenu->addAction("Save to file", this, &TxConfAdvDialog::unsignedSaveFile);
    ui->btn_exportUnsigned->setMenu(m_exportUnsignedMenu);

    m_exportSignedMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::signedCopy);
    m_exportSignedMenu->addAction("Save to file", this, &TxConfAdvDialog::signedSaveFile);
    ui->btn_exportSigned->setMenu(m_exportSignedMenu);

    if (m_ctx->currentWallet->viewOnly()) {
        ui->btn_exportSigned->hide();
        ui->btn_send->hide();
    }

    ui->label_description->setText(QString("Description: %1").arg(description));

    connect(ui->btn_sign, &QPushButton::clicked, this, &TxConfAdvDialog::signTransaction);
    connect(ui->btn_send, &QPushButton::clicked, this, &TxConfAdvDialog::broadcastTransaction);
    connect(ui->btn_close, &QPushButton::clicked, this, &TxConfAdvDialog::closeDialog);

    ui->inputs->setFont(ModelUtils::getMonospaceFont());
    ui->outputs->setFont(ModelUtils::getMonospaceFont());

    this->adjustSize();
}

void TxConfAdvDialog::setTransaction(PendingTransaction *tx) {
    ui->btn_sign->hide();

    m_tx = tx;
    m_tx->refresh();
    PendingTransactionInfo *ptx = m_tx->transaction(0); //Todo: support split transactions

    ui->txid->setText(tx->txid().first());

    ui->amount->setText(WalletManager::displayAmount(tx->amount()));
    ui->fee->setText(WalletManager::displayAmount(ptx->fee()));
    ui->total->setText(WalletManager::displayAmount(tx->amount() + ptx->fee()));

    auto size_str = [this]{
        if (m_ctx->currentWallet->viewOnly()) {
            return QString("Size: %1 bytes (unsigned)").arg(QString::number(m_tx->unsignedTxToBin().size()));
        } else {
            auto size = m_tx->signedTxToHex(0).size() / 2;
            return QString("Size: %1 bytes (%2 bytes unsigned)").arg(QString::number(size), QString::number(m_tx->unsignedTxToBin().size()));
        }
    }();
    ui->label_size->setText(size_str);

    this->setupConstructionData(ptx);
}

void TxConfAdvDialog::setUnsignedTransaction(UnsignedTransaction *utx) {
    m_utx = utx;
    m_utx->refresh();

    ui->btn_exportUnsigned->hide();
    ui->btn_exportSigned->hide();
    ui->btn_sign->show();
    ui->btn_send->hide();

    ui->txid->setText("n/a");
    ui->label_size->setText("Size: n/a");

    ui->amount->setText(WalletManager::displayAmount(utx->amount(0)));
    ui->fee->setText(WalletManager::displayAmount(utx->fee(0)));
    ui->total->setText(WalletManager::displayAmount(utx->amount(0) + utx->fee(0)));

    ConstructionInfo *ci = m_utx->constructionInfo(0);
    this->setupConstructionData(ci);
}

void TxConfAdvDialog::setupConstructionData(ConstructionInfo *ci) {
    QString inputs_str;
    auto inputs = ci->inputs();
    for (const auto& i: inputs) {
        inputs_str += QString("%1 %2\n").arg(i->pubKey(), WalletManager::displayAmount(i->amount()));
    }
    ui->inputs->setText(inputs_str);
    ui->label_inputs->setText(QString("Inputs (%1)").arg(QString::number(inputs.size())));

    auto outputs = ci->outputs();

    QTextCursor cursor = ui->outputs->textCursor();
    for (const auto& o: outputs) {
        auto address = o->address();
        auto amount = WalletManager::displayAmount(o->amount());
        cursor.insertText(address, textFormat(address));
        cursor.insertText(QString(" %1").arg(amount), QTextCharFormat());
        cursor.insertBlock();
    }
    ui->label_outputs->setText(QString("Outputs (%1)").arg(QString::number(outputs.size())));

    ui->label_ringSize->setText(QString("Ring size: %1").arg(QString::number(ci->minMixinCount() + 1)));
    ui->label_unlockTime->setText(QString("Unlock time: %1 (height)").arg(QString::number(ci->unlockTime())));
}

void TxConfAdvDialog::signTransaction() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save signed transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if(fn.isEmpty()) return;

    m_utx->sign(fn) ? QMessageBox::information(this, "Sign transaction", "Transaction saved successfully")
                    : QMessageBox::warning(this, "Sign transaction", "Failes to save transaction to file.");
}

void TxConfAdvDialog::unsignedSaveFile() {
    QString defaultName = QString("%1_unsigned_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save transaction to file", QDir::home().filePath(defaultName), "Transaction (*unsigned_monero_tx)");
    if(fn.isEmpty()) return;

    m_tx->saveToFile(fn) ? QMessageBox::information(this, "Transaction saved to file", "Transaction saved successfully")
                         : QMessageBox::warning(this, "Save transaction", "Failed to save transaction to file.");
}

void TxConfAdvDialog::signedSaveFile() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if(fn.isEmpty()) return;

    m_tx->saveToFile(fn) ? QMessageBox::information(this, "Transaction saved to file", "Transaction saved successfully")
                         : QMessageBox::warning(this, "Save transaction", "Failed to save transaction to file.");
}

void TxConfAdvDialog::unsignedQrCode() {
    if (m_tx->unsignedTxToBin().size() > 2953) {
        QMessageBox::warning(this, "Unable to show QR code", "Transaction size exceeds the maximum size for QR codes (2953 bytes).");
        return;
    }

    QrCode qr(m_tx->unsignedTxToBin(), QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::LOW);
    auto *dialog = new QrCodeDialog(this, qr, "Unsigned Transaction");
    dialog->exec();
    dialog->deleteLater();
}

void TxConfAdvDialog::unsignedCopy() {
    Utils::copyToClipboard(m_tx->unsignedTxToBase64());
}

void TxConfAdvDialog::signedCopy() {
    Utils::copyToClipboard(m_tx->signedTxToHex(0));
}

void TxConfAdvDialog::signedQrCode() {
}

void TxConfAdvDialog::broadcastTransaction() {
    if (m_tx == nullptr) return;
    m_ctx->currentWallet->commitTransactionAsync(m_tx);
    QDialog::accept();
}

void TxConfAdvDialog::closeDialog() {
    if (m_tx != nullptr)
        m_ctx->currentWallet->disposeTransaction(m_tx);
    if (m_utx != nullptr)
        m_ctx->currentWallet->disposeTransaction(m_utx);
    QDialog::reject();
}

QTextCharFormat TxConfAdvDialog::textFormat(const QString &address) {
    auto index = m_ctx->currentWallet->subaddressIndex(address);
    if (index.first == 0 && index.second == 0) {
        QTextCharFormat rec;
        rec.setBackground(QBrush(ColorScheme::YELLOW.asColor(true)));
        rec.setToolTip("Wallet change/primary address");
        return rec;
    }
    if (index.first >= 0) {
        QTextCharFormat rec;
        rec.setBackground(QBrush(ColorScheme::GREEN.asColor(true)));
        rec.setToolTip("Wallet receive address");
        return rec;
    }
    return QTextCharFormat();
}

TxConfAdvDialog::~TxConfAdvDialog() {
    delete ui;
}
