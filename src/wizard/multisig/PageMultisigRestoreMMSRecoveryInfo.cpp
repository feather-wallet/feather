// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigRestoreMMSRecoveryInfo.h"
#include "ui_PageMultisigRestoreMMSRecoveryInfo.h"

#include "WalletManager.h"

PageMultisigRestoreMMSRecoveryInfo::PageMultisigRestoreMMSRecoveryInfo(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigRestoreMMSRecoveryInfo)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Enter MMS recovery info");

}

void PageMultisigRestoreMMSRecoveryInfo::initializePage() {
    ui->mmsRecoveryInfo->setPlainText("");
}

int PageMultisigRestoreMMSRecoveryInfo::nextId() const {
    return WalletWizard::Page_SetRestoreHeight;
}

bool PageMultisigRestoreMMSRecoveryInfo::validatePage() {
    m_fields->multisigMMSRecovery = ui->mmsRecoveryInfo->toPlainText();
    return true;
}
