// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageRecoverWallet.h"


#include "WalletWizard.h"
#include "PageMenu.h"
#include "ui_PageRecoverWallet.h"

#include <QFileDialog>

#include "config-feather.h"

PageRecoverWallet::PageRecoverWallet(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageRecoverWallet)
        , m_fields(fields)
{
    ui->setupUi(this);

    this->setTitle("Recover wallet");
}

void PageRecoverWallet::initializePage() {
    ui->radio_standard->setChecked(true);
    this->setTitle(m_fields->modeText);
}

int PageRecoverWallet::nextId() const {
    if (ui->radio_standard->isChecked())
        return WalletWizard::Page_KeyType;
    if (ui->radio_viewOnly->isChecked())
        return WalletWizard::Page_WalletRestoreKeys;
    if (ui->radio_multisig->isChecked())
        return WalletWizard::Page_MultisigRestoreSeed;
    if (ui->radio_hardware->isChecked())
        return WalletWizard::Page_HardwareDevice;
    return 0;
}
