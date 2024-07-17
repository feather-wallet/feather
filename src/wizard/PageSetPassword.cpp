// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageSetPassword.h"
#include "ui_PageSetPassword.h"
#include "WalletWizard.h"

#include "utils/Icons.h"

PageSetPassword::PageSetPassword(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageSetPassword)
    , m_fields(fields)
{
    ui->setupUi(this);

    ui->frame_password->setInfo(icons()->icon("lock"), "Choose a password to encrypt your wallet keys.");

    connect(ui->widget_password, &PasswordSetWidget::passwordEntryChanged, [this]{
        this->completeChanged();
    });
}

void PageSetPassword::initializePage() {
//    bool multisig = ( || m_fields->mode == WizardMode::RestoreMultisig);
    this->setFinalPage(m_fields->mode != WizardMode::CreateMultisig);
//    this->setFinalPage(true);
    this->setButtonText(QWizard::FinishButton, "Create/Open wallet");
    this->setButtonText(QWizard::CommitButton, "Next");
    this->setCommitPage(true);
    this->setTitle(m_fields->modeText);
    ui->widget_password->resetFields();
}

bool PageSetPassword::validatePage() {
    m_fields->password = ui->widget_password->password();

    // Prevent double clicks from creating a wallet twice
    if (!m_walletCreated) {
        emit createWallet();
        m_walletCreated = true;
    }

    return true;
}

int PageSetPassword::nextId() const {
    if (m_fields->mode == WizardMode::CreateMultisig) {
        return WalletWizard::Page_MultisigCreateSetupKey;
    }

    return -1;
}

bool PageSetPassword::isComplete() const {
    return ui->widget_password->passwordsMatch();
}
