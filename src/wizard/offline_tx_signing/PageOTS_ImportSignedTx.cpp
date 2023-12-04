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
        : PageOTS_Import(parent, wallet, wizardFields, 4, "signed transaction", "Transaction (*signed_monero_tx)", "Send..")
{
}

void PageOTS_ImportSignedTx::importFromStr(const std::string &data) {
    PendingTransaction *tx = m_wallet->loadSignedTxFromStr(data);
    if (tx->status() != PendingTransaction::Status_Ok) {
        m_scanWidget->pause();
        Utils::showError(this, "Failed to import signed transaction", m_wallet->errorString());
        m_scanWidget->reset();
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
