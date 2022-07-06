// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "TxImportDialog.h"
#include "ui_TxImportDialog.h"

#include <QMessageBox>

#include "utils/NetworkManager.h"

TxImportDialog::TxImportDialog(QWidget *parent, QSharedPointer<AppContext> ctx)
        : WindowModalDialog(parent)
        , ui(new Ui::TxImportDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    connect(ui->btn_import, &QPushButton::clicked, this, &TxImportDialog::onImport);

    this->adjustSize();
}

void TxImportDialog::onImport() {
    QString txid = ui->line_txid->text();

    if (m_ctx->wallet->haveTransaction(txid)) {
        QMessageBox::warning(this, "Warning", "This transaction already exists in the wallet. "
                                              "If you can't find it in your history, "
                                              "check if it belongs to a different account (Wallet -> Account)");
        return;
    }

    if (m_ctx->wallet->importTransaction(txid)) {
        if (!m_ctx->wallet->haveTransaction(txid)) {
            QMessageBox::warning(this, "Import transaction", "This transaction does not belong to this wallet.");
            return;
        }
        QMessageBox::information(this, "Import transaction", "Transaction imported successfully.");
    } else {
        QMessageBox::warning(this, "Import transaction", "Transaction import failed.");
    }
    m_ctx->refreshModels();
}

TxImportDialog::~TxImportDialog() = default;
