// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "TxImportDialog.h"
#include "ui_TxImportDialog.h"

#include <QMessageBox>

#include "utils/NetworkManager.h"

TxImportDialog::TxImportDialog(QWidget *parent, Wallet *wallet)
        : WindowModalDialog(parent)
        , ui(new Ui::TxImportDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_import, &QPushButton::clicked, this, &TxImportDialog::onImport);

    this->adjustSize();
}

void TxImportDialog::onImport() {
    QString txid = ui->line_txid->text();

    if (m_wallet->haveTransaction(txid)) {
        Utils::showWarning(this, "Transaction already exists in wallet", "If you can't find it in your history, "
                                                                       "check if it belongs to a different account (Wallet -> Account)");
        return;
    }

    if (m_wallet->importTransaction(txid)) {
        if (!m_wallet->haveTransaction(txid)) {
            Utils::showError(this, "Unable to import transaction", "This transaction does not belong to the wallet");
            return;
        }
        Utils::showInfo(this, "Transaction imported successfully", "");
    } else {
        Utils::showError(this, "Failed to import transaction", "");
    }
    m_wallet->refreshModels();
}

TxImportDialog::~TxImportDialog() = default;
