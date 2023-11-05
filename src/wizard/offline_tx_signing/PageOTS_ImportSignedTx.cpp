// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ImportSignedTx.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>

#include "dialog/TxConfDialog.h"
#include "dialog/TxConfAdvDialog.h"
#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_ImportSignedTx::PageOTS_ImportSignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : PageOTS_Import(parent, wallet, wizardFields, "signed transaction", "Send..")
{
}

void PageOTS_ImportSignedTx::importFromStr(const std::string &data) {
    PendingTransaction *tx = m_wallet->loadSignedTxFromStr(data);
    if (tx->status() != PendingTransaction::Status_Ok) {
        Utils::showError(this, "Failed to import signed transaction", m_wallet->errorString());
        m_scanWidget->reset();
        return;
    }

    m_wizardFields->tx = tx;
    PageOTS_Import::onSuccess();
}

void PageOTS_ImportSignedTx::importFromFile() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*signed_monero_tx);;All Files (*)");
    if (fn.isEmpty()) {
        return;
    }
    
    PendingTransaction *tx = m_wallet->loadSignedTxFile(fn);
    auto err = m_wallet->errorString();
    if (!err.isEmpty()) {
        Utils::showError(this, "Failed to load signed transaction", err);
        return;
    }

    m_wizardFields->tx = tx;
    PageOTS_Import::onSuccess();
}

int PageOTS_ImportSignedTx::nextId() const {
    return -1;
}

bool PageOTS_ImportSignedTx::validatePage() {
    m_scanWidget->disconnect();
    m_wizardFields->readyToCommit = true;
    return true;
}
