// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PageSetSeedPassphrase.h"
#include "ui_PageSetSeedPassphrase.h"
#include "WalletWizard.h"

PageSetSeedPassphrase::PageSetSeedPassphrase(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageSetSeedPassphrase)
    , m_fields(fields)
{
    ui->setupUi(this);

    this->setTitle("Seed Passphrase");
}

void PageSetSeedPassphrase::initializePage() {
    ui->linePassphrase->setText("");
}

bool PageSetSeedPassphrase::validatePage() {
    m_fields->seedOffsetPassphrase = ui->linePassphrase->text();
    return true;
}

int PageSetSeedPassphrase::nextId() const {
    if (m_fields->showSetSubaddressLookaheadPage) {
        return WalletWizard::Page_SetSubaddressLookahead;
    }

    return WalletWizard::Page_WalletFile;
}