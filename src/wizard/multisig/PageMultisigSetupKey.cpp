// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSetupKey.h"
#include "ui_PageMultisigSetupKey.h"

#include "MultisigMessageStore.h"

#include "utils/Icons.h"
#include "utils/Utils.h"

PageMultisigSetupKey::PageMultisigSetupKey(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSetupKey)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Setup key");

    ui->infoFrame->setInfo(icons()->icon("key.png"), "Share the setup key with other cosigners over a secure channel "
                                                     "(e.g. end-to-end encrypted group chat.)\n\nDo not share the setup "
                                                     "key with people that aren't intended cosigners for this wallet.");

    // Break at the end of the textbox
    ui->line_setupKey->setWordWrapMode(QTextOption::WrapAnywhere);

    connect(ui->btn_copy, &QPushButton::clicked, [this] {
        Utils::copyToClipboard(ui->line_setupKey->toPlainText());
    });
}

void PageMultisigSetupKey::initializePage() {
    ui->line_setupKey->clear();

    // We currently only support automatic and manual mode.
    MultisigMessageStore::SetupMode mode = m_fields->multisigAutomaticSetup ? MultisigMessageStore::AUTOMATIC : MultisigMessageStore::MANUAL;

    m_setupKey = m_fields->wallet->mmsStore()->createSetupKey(m_fields->multisigThreshold, m_fields->multisigSigners, m_fields->multisigService, m_fields->multisigChannel, mode);
    if (m_setupKey.isEmpty()) {
        Utils::showError(this, "Unable to create setup key", "Unknown error");
        return;
    }

    ui->line_setupKey->setPlainText(m_setupKey);
    m_keyGenerated = true;
}

int PageMultisigSetupKey::nextId() const {
    return WalletWizard::Page_MultisigEnterName;
}

bool PageMultisigSetupKey::validatePage() {
    if (!m_keyGenerated) {
        Utils::showError(this, "Unable to proceed", "No setup key was generated", {"You have found a bug. Please contact the developers."}, "report_an_issue");
        return false;
    }

    m_fields->multisigSetupKey = m_setupKey;
    return true;
}
