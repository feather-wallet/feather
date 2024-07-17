// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigParticipants.h"
#include "ui_PageMultisigParticipants.h"

#include "utils/Icons.h"
#include "utils/Utils.h"

PageMultisigParticipants::PageMultisigParticipants(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigParticipants)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Create setup key (1/3)");

    ui->infoFrame->setInfo(icons()->icon("sign.png"), "Choose the number of participants (cosigners) and the number of signatures required to spend funds.");

    connect(ui->spin_signers, &QSpinBox::valueChanged, [this](int value){
       ui->spin_threshold->setMaximum(value);
    });
}


void PageMultisigParticipants::initializePage() {
    ui->spin_threshold->setValue(2);
    ui->spin_signers->setValue(3);
}

int PageMultisigParticipants::nextId() const {
    return WalletWizard::Page_MultisigEnterChannel;
}

bool PageMultisigParticipants::validatePage() {
    int threshold = ui->spin_threshold->value();

    if (threshold < 2) {
        const auto button = QMessageBox::question(this, "Insecure multisig threshold", "Requiring only 1 signature means any cosigner can spend from this wallet.\n\nAre you sure you want to create this setup key?");
        if (button == QMessageBox::No) {
            return false;
        }
    }

    m_fields->multisigThreshold = threshold;
    m_fields->multisigSigners = ui->spin_signers->value();
    return true;
}
