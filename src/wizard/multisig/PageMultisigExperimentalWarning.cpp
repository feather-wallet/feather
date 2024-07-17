// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigExperimentalWarning.h"
#include "ui_PageMultisigExperimentalWarning.h"

#include <QFileDialog>

#include "libwalletqt/MultisigMessageStore.h"
#include "utils/Icons.h"

PageMultisigExperimentalWarning::PageMultisigExperimentalWarning(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigExperimentalWarning)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Experimental Feature Warning");

    ui->warningFrame->setInfo(icons()->icon("warning.png"), "This is an experimental feature.");

    ui->label_warning->setText("Monero's multisig implementation has not been audited.<br><br>"
                               "It might be fundamentally broken, which could result in a <u>loss of funds</u>.<br><br>"
                               "You are strongly advised to only test this feature with cosigners you trust.");
    ui->label_warning->setTextFormat(Qt::RichText);

    connect(ui->check_confirm, &QCheckBox::clicked, [this](bool checked){
       completeChanged();
    });
}

void PageMultisigExperimentalWarning::initializePage() {
    ui->check_confirm->setChecked(false);
}

int PageMultisigExperimentalWarning::nextId() const {
    return WalletWizard::Page_WalletFile;
}

bool PageMultisigExperimentalWarning::validatePage() {
    return true;
}

bool PageMultisigExperimentalWarning::isComplete() const {
    if (ui->check_confirm->isChecked()) {
        return true;
    }

    return false;
}

