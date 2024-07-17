// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSignerConfig.h"
#include "ui_PageMultisigSignerConfig.h"

#include "utils/Icons.h"

PageMultisigSignerConfig::PageMultisigSignerConfig(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSignerConfig)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Create setup key (3/3)");

    ui->infoFrame->setInfo(icons()->icon("sign.png"), "Choose the signer configuration mode.");
}

void PageMultisigSignerConfig::initializePage() {
    ui->radio_automatic->setChecked(true);
}

int PageMultisigSignerConfig::nextId() const {
    return WalletWizard::Page_MultisigShowSetupKey;
}

bool PageMultisigSignerConfig::validatePage() {
    m_fields->multisigAutomaticSetup = ui->radio_automatic->isChecked();
    return true;
}

