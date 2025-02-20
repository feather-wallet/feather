// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

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
    this->setFinalPage(true);

    ui->frame_password->setInfo(icons()->icon("lock"), "Choose a password to encrypt your wallet keys.");

    connect(ui->widget_password, &PasswordSetWidget::passwordEntryChanged, [this]{
        this->completeChanged();
    });

    this->setButtonText(QWizard::FinishButton, "Create/Open wallet");
}

void PageSetPassword::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->widget_password->resetFields();
}

bool PageSetPassword::validatePage() {
    m_fields->password = ui->widget_password->password();
    emit createWallet();
    return true;
}

int PageSetPassword::nextId() const {
    return -1;
}

bool PageSetPassword::isComplete() const {
    return ui->widget_password->passwordsMatch();
}
