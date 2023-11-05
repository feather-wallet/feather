// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ImportKeyImages.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>

#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_ImportKeyImages::PageOTS_ImportKeyImages(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : PageOTS_Import(parent, wallet, wizardFields, "key images", "Create transaction")
{
}

void PageOTS_ImportKeyImages::importFromStr(const std::string &data) {
    bool r = m_wallet->importKeyImagesFromStr(data);
    if (!r) {
        Utils::showError(this, "Failed to import key images", m_wallet->errorString());
        m_scanWidget->reset();
        return;
    }

    PageOTS_Import::onSuccess();
}

void PageOTS_ImportKeyImages::importFromFile() {
    QString fn = QFileDialog::getOpenFileName(this, "Import key image file", QDir::homePath(), "Key Images (*_keyImages);;All Files (*)");
    if (fn.isEmpty()) {
        return;
    }
    
    bool r = m_wallet->importKeyImages(fn);
    if (!r) {
        Utils::showError(this, "Failed to import key images", m_wallet->errorString());
        return;
    }

    PageOTS_Import::onSuccess();
}

int PageOTS_ImportKeyImages::nextId() const {
    return -1;
}
