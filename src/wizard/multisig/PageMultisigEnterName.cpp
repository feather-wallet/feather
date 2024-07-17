// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigEnterName.h"
#include "ui_PageMultisigEnterName.h"

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "libwalletqt/MultisigMessageStore.h"

PageMultisigEnterName::PageMultisigEnterName(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigEnterName)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Enter your name");

    ui->infoFrame->setInfo(icons()->icon("change_account.png"), "Enter your (user)name, so other participants "
                                                                "can identify you.\n\nYour name will not become known "
                                                                "to the messaging service.");

    // We can't change the service URL or username after the setup has started.
    this->setCommitPage(true);
    this->setButtonText(QWizard::CommitButton, "Next");
}

void PageMultisigEnterName::initializePage() {
    ui->line_name->setText("");
}

int PageMultisigEnterName::nextId() const {
    if (m_fields->multisigAutomaticSetup) {
      return WalletWizard::Page_MultisigSetupWallet;
    }
    else {
      return WalletWizard::Page_MultisigOwnAddress;
    }
}

bool PageMultisigEnterName::validatePage() {
    QString username = ui->line_name->text();
    if (username.isEmpty()) {
        Utils::showError(this, "Enter a name to continue");
        return false;
    }

    m_fields->multisigUsername = username;

    // We now have all the information needed to init the MMS
    if (m_fields->mode == WizardMode::CreateMultisig) {
        m_fields->wallet->mmsStore()->init(m_fields->multisigSetupKey, username);
    }

    return true;
}
