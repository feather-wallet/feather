// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ImportUnsignedTx.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>

#include "dialog/TxConfAdvDialog.h"
#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_ImportUnsignedTx::PageOTS_ImportUnsignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : PageOTS_Import(parent, wallet, wizardFields, "unsigned transaction", "Review transaction")
{
}

void PageOTS_ImportUnsignedTx::importFromStr(const std::string &data) {
    UnsignedTransaction *utx = m_wallet->loadUnsignedTransactionFromStr(data);

    if (utx->status() != UnsignedTransaction::Status_Ok) {
        Utils::showError(this, "Failed to import unsigned transaction", m_wallet->errorString());
        m_scanWidget->reset();
        return;
    }

    m_wizardFields->utx = utx;
    PageOTS_Import::onSuccess();
}

void PageOTS_ImportUnsignedTx::importFromFile() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*unsigned_monero_tx);;All Files (*)");
    if (fn.isEmpty()) {
        return;
    }
    
    UnsignedTransaction *utx = m_wallet->loadTxFile(fn);
    if (utx->status() != UnsignedTransaction::Status_Ok) {
        Utils::showError(this, "Failed to import unsigned transaction", m_wallet->errorString());
        m_scanWidget->reset();
        return;
    }

    m_wizardFields->utx = utx;
    PageOTS_Import::onSuccess();
}


int PageOTS_ImportUnsignedTx::nextId() const {
    return OfflineTxSigningWizard::Page_ExportSignedTx;
}


//bool PageOTS_ImportUnsignedTx::validatePage() {
//    m_scanWidget->disconnect();
//    TxConfAdvDialog dialog{m_wallet, "", this};
//    dialog.setUnsignedTransaction(m_wizardFields->utx);
//    auto r = dialog.exec();
//    
//    if (r != QDialog::Accepted) {
//        return false;
//    }
//
//    // TODO: error handling
//    m_wizardFields->utx->signToStr(m_wizardFields->signedTx);
//    return true;
//}