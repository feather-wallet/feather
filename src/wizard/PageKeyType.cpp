// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageKeyType.h"
#include "WalletWizard.h"
#include "ui_PageKeyType.h"

#include <QFileDialog>


PageKeyType::PageKeyType(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageKeyType)
        , m_fields(fields)
{
    ui->setupUi(this);
//    this->setTitle("Restore multisig wallet");
    this->setTitle("Recover wallet");
}

void PageKeyType::initializePage() {

}

int PageKeyType::nextId() const {
    if (ui->radio_seed) {
        return WalletWizard::Page_WalletRestoreSeed;
    } else {
        return WalletWizard::Page_WalletRestoreKeys;
    }

    return 0;
}

bool PageKeyType::validatePage() {
    m_fields->clearFields();

    return true;
}