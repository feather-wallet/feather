// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ImportOffline.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>

#include "dialog/TxConfAdvDialog.h"
#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_ImportOffline::PageOTS_ImportOffline(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : PageOTS_Import(parent, wallet, wizardFields, "outputs or unsigned transactions", "Next")
{
}

void PageOTS_ImportOffline::importFromStr(const std::string &data) {
    if (this->isOutputs(data)) {
        bool r = m_wallet->importOutputsFromStr(data);
        if (!r) {
            Utils::showError(this, "Failed to import outputs", m_wallet->errorString());
            m_scanWidget->reset();
            return;
        }

        ui->frame_status->show();
        ui->frame_status->setInfo(icons()->icon("confirmed.svg"), "Outputs imported successfully");
    }
    else if (this->isUnsignedTransaction(data)) {
        UnsignedTransaction *utx = m_wallet->loadUnsignedTransactionFromStr(data);

        if (utx->status() != UnsignedTransaction::Status_Ok) {
            Utils::showError(this, "Failed to import unsigned transaction", m_wallet->errorString());
            m_scanWidget->reset();
            return;
        }

        ui->frame_status->show();
        ui->frame_status->setInfo(icons()->icon("confirmed.svg"), "Unsigned transaction imported successfully");
        m_wizardFields->utx = utx;
    }
    else {
        Utils::showError(this, "Failed to import outputs or unsigned transaction", "Unrecognized data");
        return;
    }

    PageOTS_Import::onSuccess();
}

void PageOTS_ImportOffline::importFromFile() {
    QString fn = QFileDialog::getOpenFileName(this, "Import outputs or unsigned tx file", QDir::homePath(), "All Files (*)");
    if (fn.isEmpty()) {
        return;
    }

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray qdata = file.readAll();
    std::string data = qdata.toStdString();

    file.close();

    importFromStr(data);
}


bool PageOTS_ImportOffline::isOutputs(const std::string &data) {
    std::string outputMagic = "Monero output export";
    const size_t magiclen = outputMagic.length();
    if (data.size() < magiclen || memcmp(data.data(), outputMagic.data(), magiclen) != 0) {
        return false;
    }
    m_importType = ImportType::OUTPUTS;
    return true;
}

bool PageOTS_ImportOffline::isUnsignedTransaction(const std::string &data) {
    std::string utxMagic = "Monero unsigned tx set";
    const size_t magiclen = utxMagic.length();
    if (data.size() < magiclen || memcmp(data.data(), utxMagic.data(), magiclen) != 0) {
        return false;
    }
    m_importType = ImportType::UNSIGNED_TX;
    return true;
}


int PageOTS_ImportOffline::nextId() const {
    return m_importType == ImportType::OUTPUTS ? OfflineTxSigningWizard::Page_ExportKeyImages : OfflineTxSigningWizard::Page_ExportSignedTx;
}

//bool PageOTS_ImportOffline::validatePage() {
//    m_scanWidget->disconnect();
//    
//    if (m_wizardFields->utx) {
////        TxConfAdvDialog dialog{m_wallet, "", this};
////        dialog.setUnsignedTransaction(m_wizardFields->utx);
////        auto r = dialog.exec();
////
////        if (r != QDialog::Accepted) {
////            return false;
////        }
//
//        m_wizardFields->utx->signToStr(m_wizardFields->signedTx);
//    }
//    
//    return true;
//}