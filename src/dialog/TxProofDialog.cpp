// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "TxProofDialog.h"
#include "ui_TxProofDialog.h"

#include <QMessageBox>

#include "libwalletqt/Transfer.h"
#include "utils/utils.h"
#include "utils/Icons.h"

TxProofDialog::TxProofDialog(QWidget *parent, QSharedPointer<AppContext> ctx, TransactionInfo *txInfo)
    : QDialog(parent)
    , ui(new Ui::TxProofDialog)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    m_txid = txInfo->hash();
    m_txKey = m_ctx->wallet->getTxKey(m_txid);
    m_direction = txInfo->direction();

    for (auto const &t: txInfo->transfers()) {
        m_OutDestinations.push_back(t->address());
    }

    for (auto const &s: txInfo->subaddrIndex()) {
        m_InDestinations.push_back(m_ctx->wallet->address(txInfo->subaddrAccount(), s));
    }

    // Due to some logic in core we can't create OutProofs
    // for churn transactions that sweep from and send to the same address
    for (auto const &address : m_InDestinations) {
        m_OutDestinations.removeAll(address);
    }

    connect(ui->radio_SpendProof, &QRadioButton::toggled, this, &TxProofDialog::selectSpendProof);
    connect(ui->radio_OutProof, &QRadioButton::toggled, this, &TxProofDialog::selectOutProof);
    connect(ui->radio_InProof, &QRadioButton::toggled, this, &TxProofDialog::selectInProof);

    connect(ui->btn_getFormattedProof, &QPushButton::pressed, this, &TxProofDialog::getFormattedProof);
    connect(ui->btn_getSignature, &QPushButton::pressed, this, &TxProofDialog::getSignature);

    ui->radio_SpendProof->setChecked(true);
    ui->label_txid->setText(m_txid);

    ui->btn_copyAddress->setIcon(icons()->icon("copy.png"));
    connect(ui->btn_copyAddress, &QPushButton::clicked, [this]{
        Utils::copyToClipboard(ui->combo_address->currentText());
    });
    ui->group_summary->hide(); // todo

    this->adjustSize();
}

void TxProofDialog::setTxId(const QString &txid) {
    ui->label_txid->setText(txid);
}

void TxProofDialog::selectSpendProof() {
    m_mode = Mode::SpendProof;
    this->resetFrames();

    if (m_direction == TransactionInfo::Direction_In) {
        this->showWarning("Your wallet did not construct this transaction. Creating a SpendProof is not possible.");
        return;
    }

    ui->frame_message->show();
    ui->label_summary->setText("This proof shows you created a transaction with the txid shown above.");
}

void TxProofDialog::selectOutProof() {
    m_mode = Mode::OutProof;
    this->resetFrames();

    if (m_OutDestinations.empty()) {
        this->showWarning("This transaction did not spend any outputs owned by this wallet. Creating an OutProof is not possible.");
        return;
    }

    if (m_txKey.isEmpty()) {
        this->showWarning("No transaction key stored for this transaction. Creating an OutProof is not possible.");
        return;
    }

    this->selectTxProof();
    ui->combo_address->addItems(m_OutDestinations);
    ui->label_summary->setText("This proof shows you paid x XMR to the address selected above.");
}

void TxProofDialog::selectInProof() {
    m_mode = Mode::InProof;
    this->resetFrames();

    if (m_InDestinations.empty()) {
        this->showWarning("Your wallet did not receive any outputs in this transaction.");
        return;
    }

    this->selectTxProof();
    ui->combo_address->addItems(m_InDestinations);
    ui->label_summary->setText("This proof shows you received x XMR to the address selected above.");
}

void TxProofDialog::selectTxProof() {
    ui->frame_txKeyWarning->hide();
    ui->frame_message->show();
    ui->frame_address->show();
    ui->combo_address->clear();
}

void TxProofDialog::resetFrames() {
    ui->frame_txKeyWarning->hide();
    ui->frame_message->hide();
    ui->frame_address->hide();
    this->toggleButtons(true);
}

void TxProofDialog::toggleButtons(bool enabled) {
    ui->btn_getFormattedProof->setEnabled(enabled);
    ui->btn_getSignature->setEnabled(enabled);
}

void TxProofDialog::showWarning(const QString &message) {
    this->toggleButtons(false);
    ui->frame_txKeyWarning->show();
    ui->label_txKeyWarning->setText(message);
}

void TxProofDialog::getFormattedProof() {
    QString message = ui->message->toPlainText();
    QString address = ui->combo_address->currentText();
    QString nettype = Utils::QtEnumToString(m_ctx->wallet->nettype()).toLower();
    nettype = nettype.replace(0, 1, nettype[0].toUpper()); // Capitalize first letter

    TxProof proof = this->getProof();

    if (!proof.error.isEmpty()) {
        QMessageBox::warning(this, "Get formatted proof", QString("Failed to get proof signature: %1").arg(proof.error));
        return;
    }

    QStringList signatureSplit;
    for (int i = 0; i < proof.proof.length(); i += 64) {
        signatureSplit.append(proof.proof.mid(i, 64));
    }
    QString signature = signatureSplit.join('\n');

    QString formattedProof = [this, nettype, message, address, signature]{
        switch (m_mode) {
            case Mode::SpendProof: {
                return QString("-----BEGIN SPENDPROOF-----\n"
                               "Network: Monero %1\n"
                               "Txid: %2\n"
                               "\n"
                               "%3\n"
                               "-----BEGIN SPENDPROOF SIGNATURE-----\n"
                               "\n"
                               "%4\n"
                               "-----END SPENDPROOF SIGNATURE-----").arg(nettype, m_txid, message, signature);
            }
            case Mode::OutProof: {
                return QString("-----BEGIN OUTPROOF-----\n"
                               "Network: Monero %1\n"
                               "Txid: %2\n"
                               "Address: %3\n"
                               "\n"
                               "%4\n"
                               "-----BEGIN OUTPROOF SIGNATURE-----\n"
                               "\n"
                               "%5\n"
                               "-----END OUTPROOF SIGNATURE-----").arg(nettype, m_txid, address, message, signature);
            }
            case Mode::InProof: {
                return QString("-----BEGIN INPROOF-----\n"
                               "Network: Monero %1\n"
                               "Txid: %2\n"
                               "Address: %3\n"
                               "\n"
                               "%4\n"
                               "-----BEGIN INPROOF SIGNATURE-----\n"
                               "\n"
                               "%5\n"
                               "-----END INPROOF SIGNATURE-----").arg(nettype, m_txid, address, message, signature);
            }
            default:
                return QString("");
        }
    }();

    Utils::copyToClipboard(formattedProof);
    QMessageBox::information(this, "Get formatted proof", "Formatted proof copied to clipboard");
}

void TxProofDialog::getSignature() {
    TxProof proof = this->getProof();

    if (!proof.error.isEmpty()) {
        QMessageBox::warning(this, "Get proof signature", QString("Failed to get proof signature: %1").arg(proof.error));
        return;
    }

    Utils::copyToClipboard(proof.proof);
    QMessageBox::information(this, "Get proof singature", "Proof signature copied to clipboard");
}

TxProof TxProofDialog::getProof() {
    QString message = ui->message->toPlainText();
    QString address = ui->combo_address->currentText();

    TxProof proof = [this, message, address]{
        switch (m_mode) {
            case Mode::SpendProof: {
                return m_ctx->wallet->getSpendProof(m_txid, message);
            }
            case Mode::OutProof:
            case Mode::InProof:
            default: { // Todo: split this into separate functions
                return m_ctx->wallet->getTxProof(m_txid, address, message);
            }
        }
    }();

    return proof;
}

TxProofDialog::~TxProofDialog() {
    delete ui;
}

