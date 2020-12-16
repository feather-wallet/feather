#include "txproofwidget.h"
#include "ui_txproofwidget.h"

#include <QMessageBox>
#include <utility>

#include "utils/utils.h"

TxProofWidget::TxProofWidget(QWidget *parent, Wallet *wallet, TransactionInfo *txInfo)
    : QWidget(parent)
    , ui(new Ui::TxProofWidget)
    , m_wallet(wallet)
    , m_txid(txInfo->hash())
{
    ui->setupUi(this);

    if (txInfo->direction() == TransactionInfo::Direction_Out) {
        for (auto const &d: txInfo->destinations()) {
            ui->comboBox_TxProofAddresses->addItem(d);
        }
    } else {
        ui->btn_copySpendProof->setEnabled(false);

        for (auto const &s: txInfo->subaddrIndex()) {
            ui->comboBox_TxProofAddresses->addItem(wallet->address(txInfo->subaddrAccount(), s));
        }
    }

    if (ui->comboBox_TxProofAddresses->count() == 0) {
        ui->btn_copyTxProof->setEnabled(false);
    }

    connect(ui->btn_copySpendProof, &QPushButton::clicked, this, &TxProofWidget::copySpendProof);
    connect(ui->btn_copyTxProof, &QPushButton::clicked, this, &TxProofWidget::copyTxProof);

    ui->label_SpendProof->setHelpText("A SpendProof proves authorship of a transaction.\n\n"
                                            "SpendProofs do not prove that a particular amount was sent to an address, for that use OutProofs.");
    ui->label_InOutProof->setHelpText("An InProof proves ownership of an output.\n"
                                            "An OutProof shows the prover sent an output to an address.");
}

void TxProofWidget::copySpendProof() {
    auto txproof = m_wallet->getSpendProof(m_txid, "");
    if (!txproof.error.isEmpty()) {
        QMessageBox::warning(this, "Copy SpendProof", QString("Failed to copy SpendProof").arg(txproof.error));
        return;
    }

    Utils::copyToClipboard(txproof.proof);
    QMessageBox::information(this, "Copy SpendProof", "SpendProof copied to clipboard");
}

void TxProofWidget::copyTxProof() {
    auto txproof = m_wallet->getTxProof(m_txid, ui->comboBox_TxProofAddresses->currentText(), "");
    if (!txproof.error.isEmpty()) {
        QMessageBox::warning(this, "Copy Transaction Proof", QString("Failed to copy transaction proof: %1").arg(txproof.error));
        return;
    }

    Utils::copyToClipboard(txproof.proof);
    QMessageBox::information(this, "Copy Transaction Proof", "Transaction proof copied to clipboard");
}

TxProofWidget::~TxProofWidget() {
    delete ui;
}
