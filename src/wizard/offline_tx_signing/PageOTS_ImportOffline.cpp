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
        : PageOTS_Import(parent, wallet, wizardFields, 1, "outputs or unsigned transactions", "All Files (*)", "Next")
{
}

void PageOTS_ImportOffline::importFromStr(const std::string &data) {
    Utils::Message message{this, Utils::ERROR};

    if (this->isOutputs(data)) {
        std::string keyImages;
        bool r = m_wallet->exportKeyImagesForOutputsFromStr(data, keyImages);
        if (!r) {
            m_scanWidget->pause();
            message.title = "Failed to import outputs";
            QString error = m_wallet->errorString();
            message.description = error;
            if (error.contains("Failed to decrypt")) {
                message.helpItems = {"You may have opened the wrong view-only wallet."};
            }
            Utils::showMsg(message);
            m_scanWidget->reset();
            return;
        }

        m_wizardFields->keyImages = keyImages;
        ui->frame_status->show();
        ui->frame_status->setInfo(icons()->icon("confirmed.svg"), "Outputs imported successfully");
    }
    else if (this->isUnsignedTransaction(data)) {
        UnsignedTransaction *utx = m_wallet->loadUnsignedTransactionFromStr(data);

        if (utx->status() != UnsignedTransaction::Status_Ok) {
            m_scanWidget->pause();
            message.title = "Failed to import unsigned transaction";
            QString error = m_wallet->errorString();
            message.description = error;
            if (error.contains("Failed to decrypt")) {
                message.helpItems = {"You may have opened the wrong view-only wallet."};
            }
            Utils::showMsg(message);
            m_scanWidget->reset();
            return;
        }

        ui->frame_status->show();
        ui->frame_status->setInfo(icons()->icon("confirmed.svg"), "Unsigned transaction imported successfully");
        m_wizardFields->utx = utx;
        m_wizardFields->readyToSign = true;
    }
    else {
        Utils::showError(this, "Failed to import outputs or unsigned transaction", "Unrecognized data");
        return;
    }

    PageOTS_Import::onSuccess();
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
    return m_importType == ImportType::OUTPUTS ? OfflineTxSigningWizard::Page_ExportKeyImages : OfflineTxSigningWizard::Page_SignTx;
}
