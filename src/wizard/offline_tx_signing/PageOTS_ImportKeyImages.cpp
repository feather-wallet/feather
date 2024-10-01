// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageOTS_ImportKeyImages.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QCheckBox>

#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_ImportKeyImages::PageOTS_ImportKeyImages(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : PageOTS_Import(parent, wallet, wizardFields, 2, "key images", "Key Images (*_keyImages)", "Create transaction")
{
}

void PageOTS_ImportKeyImages::importFromStr(const std::string &data) {
    if (!proceed()) {
        m_scanWidget->reset();
        return;
    }
    
    bool r = m_wallet->importKeyImagesFromStr(data);
    if (!r) {
        m_scanWidget->pause();
        Utils::showError(this, "Failed to import key images", m_wallet->errorString());
        m_scanWidget->reset();
        return;
    }

    PageOTS_Import::onSuccess();
}

bool PageOTS_ImportKeyImages::proceed() {
    if (!conf()->get(Config::warnOnKiImport).toBool()) {
        return true;
    }
    
    QMessageBox warning{this};
    warning.setWindowTitle("Warning");
    warning.setText("Key image import reveals which outputs you own to the node. "
                    "Make sure you are connected to a trusted node.\n\n"
                    "Do you want to proceed?");
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    switch(warning.exec()) {
        case QMessageBox::No:
            return false;
        default:
            conf()->set(Config::warnOnKiImport, false);
            return true;
    }
}

int PageOTS_ImportKeyImages::nextId() const {
    return -1;
}
