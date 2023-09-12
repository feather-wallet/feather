// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TxConfAdvDialog.h"
#include "ui_TxConfAdvDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "constants.h"
#include "dialog/QrCodeDialog.h"
#include "libwalletqt/Input.h"
#include "libwalletqt/Transfer.h"
#include "libwalletqt/WalletManager.h"
#include "qrcode/QrCode.h"
#include "utils/AppData.h"
#include "utils/config.h"
#include "utils/Utils.h"

TxConfAdvDialog::TxConfAdvDialog(Wallet *wallet, const QString &description, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::TxConfAdvDialog)
    , m_wallet(wallet)
    , m_exportUnsignedMenu(new QMenu(this))
    , m_exportSignedMenu(new QMenu(this))
    , m_exportTxKeyMenu(new QMenu(this))
{
    ui->setupUi(this);

    m_exportUnsignedMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::unsignedCopy);
    m_exportUnsignedMenu->addAction("Show as QR code", this, &TxConfAdvDialog::unsignedQrCode);
    m_exportUnsignedMenu->addAction("Save to file", this, &TxConfAdvDialog::unsignedSaveFile);
    ui->btn_exportUnsigned->setMenu(m_exportUnsignedMenu);

    m_exportSignedMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::signedCopy);
    m_exportSignedMenu->addAction("Save to file", this, &TxConfAdvDialog::signedSaveFile);
    ui->btn_exportSigned->setMenu(m_exportSignedMenu);

    m_exportTxKeyMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::txKeyCopy);
    ui->btn_exportTxKey->setMenu(m_exportTxKeyMenu);

    ui->line_description->setText(description);

    connect(ui->btn_sign, &QPushButton::clicked, this, &TxConfAdvDialog::signTransaction);
    connect(ui->btn_send, &QPushButton::clicked, this, &TxConfAdvDialog::broadcastTransaction);
    connect(ui->btn_close, &QPushButton::clicked, this, &TxConfAdvDialog::closeDialog);

    ui->amount->setFont(Utils::getMonospaceFont());
    ui->fee->setFont(Utils::getMonospaceFont());
    ui->total->setFont(Utils::getMonospaceFont());

    ui->inputs->setFont(Utils::getMonospaceFont());
    ui->outputs->setFont(Utils::getMonospaceFont());

    this->adjustSize();
}

void TxConfAdvDialog::setTransaction(PendingTransaction *tx, bool isSigned) {
    ui->btn_sign->hide();

    if (!isSigned) {
        ui->btn_exportSigned->hide();
        ui->btn_send->hide();
    }

    m_tx = tx;
    m_tx->refresh();
    PendingTransactionInfo *ptx = m_tx->transaction(0); //Todo: support split transactions

    // TODO: implement hasTxKey()
    if (!m_wallet->isHwBacked() && m_tx->transaction(0)->txKey() == "0100000000000000000000000000000000000000000000000000000000000000") {
        ui->btn_exportTxKey->hide();
    }

    m_txid = tx->txid().first();
    ui->txid->setText(m_txid);

    this->setAmounts(tx->amount(), tx->fee());

    auto size_str = [this, isSigned]{
        if (isSigned) {
            auto size = m_tx->signedTxToHex(0).size() / 2;
            return QString("Size: %1 bytes (%2 bytes unsigned)").arg(QString::number(size), QString::number(m_tx->unsignedTxToBin().size()));
        } else {

            return QString("Size: %1 bytes (unsigned)").arg(QString::number(m_tx->unsignedTxToBin().size()));
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
    ui->btn_exportTxKey->hide();
    ui->btn_sign->show();
    ui->btn_send->hide();

    ui->txid->setText("n/a");
    ui->label_size->setText("Size: n/a");

    this->setAmounts(utx->amount(0), utx->fee(0));

    ConstructionInfo *ci = m_utx->constructionInfo(0);
    this->setupConstructionData(ci);
}

void TxConfAdvDialog::setAmounts(quint64 amount, quint64 fee) {
    QString preferredCur = conf()->get(Config::preferredFiatCurrency).toString();

    auto convert = [preferredCur](double amount){
        return QString::number(appData()->prices.convert("XMR", preferredCur, amount), 'f', 2);
    };

    QString amount_str = WalletManager::displayAmount(amount);
    QString fee_str = WalletManager::displayAmount(fee);
    QString total = WalletManager::displayAmount(amount + fee);
    QVector<QString> amounts = {amount_str, fee_str, total};
    int maxLength = Utils::maxLength(amounts);
    std::for_each(amounts.begin(), amounts.end(), [maxLength](QString& amount){amount = amount.rightJustified(maxLength, ' ');});

    QString amount_fiat = convert(amount / constants::cdiv);
    QString fee_fiat = convert(fee / constants::cdiv);
    QString total_fiat = convert((amount + fee) / constants::cdiv);
    QVector<QString> amounts_fiat = {amount_fiat, fee_fiat, total_fiat};
    int maxLengthFiat = Utils::maxLength(amounts_fiat);
    std::for_each(amounts_fiat.begin(), amounts_fiat.end(), [maxLengthFiat](QString& amount){amount = amount.rightJustified(maxLengthFiat, ' ');});

    ui->amount->setText(QString("%1 (%2 %3)").arg(amounts[0], amounts_fiat[0], preferredCur));
    ui->fee->setText(QString("%1 (%2 %3)").arg(amounts[1], amounts_fiat[1], preferredCur));
    ui->total->setText(QString("%1 (%2 %3)").arg(amounts[2], amounts_fiat[2], preferredCur));
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
        auto index = m_wallet->subaddressIndex(address);
        cursor.insertText(address, Utils::addressTextFormat(index, o->amount()));
        cursor.insertText(QString(" %1").arg(amount), QTextCharFormat());
        cursor.insertBlock();
    }
    ui->label_outputs->setText(QString("Outputs (%1)").arg(QString::number(outputs.size())));

    ui->label_ringSize->setText(QString("Ring size: %1").arg(QString::number(ci->minMixinCount() + 1)));
}

void TxConfAdvDialog::signTransaction() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save signed transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }

    bool success = m_utx->sign(fn);

    if (success) {
        Utils::showInfo(this, "Transaction saved successfully");
    } else {
        Utils::showError(this, "Failed to save transaction to file");
    }
}

void TxConfAdvDialog::unsignedSaveFile() {
    QString defaultName = QString("%1_unsigned_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save transaction to file", QDir::home().filePath(defaultName), "Transaction (*unsigned_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }

    bool success = m_tx->saveToFile(fn);

    if (success) {
        Utils::showInfo(this, "Transaction saved successfully");
    } else {
        Utils::showError(this, "Failed to save transaction to file");
    }
}

void TxConfAdvDialog::signedSaveFile() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }

    bool success = m_tx->saveToFile(fn);

    if (success) {
        Utils::showInfo(this, "Transaction saved successfully");
    } else {
        Utils::showError(this, "Failed to save transaction to file");
    }
}

void TxConfAdvDialog::unsignedQrCode() {
    if (m_tx->unsignedTxToBin().size() > 2953) {
        Utils::showError(this, "Unable to show QR code", "Transaction size exceeds the maximum size for QR codes (2953 bytes)");
        return;
    }

    QrCode qr(m_tx->unsignedTxToBin(), QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::LOW);
    QrCodeDialog dialog{this, &qr, "Unsigned Transaction"};
    dialog.exec();
}

void TxConfAdvDialog::unsignedCopy() {
    Utils::copyToClipboard(m_tx->unsignedTxToBase64());
}

void TxConfAdvDialog::signedCopy() {
    Utils::copyToClipboard(m_tx->signedTxToHex(0));
}

void TxConfAdvDialog::txKeyCopy() {
    if (m_wallet->isHwBacked()) {
        Utils::showError(this, "Unable to copy transaction private key", "Function not supported for hardware wallets");
        return;
    }

    Utils::copyToClipboard(m_tx->transaction(0)->txKey());
}

void TxConfAdvDialog::signedQrCode() {
}

void TxConfAdvDialog::broadcastTransaction() {
    if (m_tx == nullptr) return;
    m_wallet->commitTransaction(m_tx, ui->line_description->text());
    QDialog::accept();
}

void TxConfAdvDialog::closeDialog() {
    if (m_tx != nullptr)
        m_wallet->disposeTransaction(m_tx);
    if (m_utx != nullptr)
        m_wallet->disposeTransaction(m_utx);
    QDialog::reject();
}

TxConfAdvDialog::~TxConfAdvDialog() = default;