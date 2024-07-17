// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigCreateSetupKey.h"
#include "ui_PageMultisigCreateSetupKey.h"

#include <QFileDialog>

#include "libwalletqt/MultisigMessageStore.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"

PageMultisigCreateSetupKey::PageMultisigCreateSetupKey(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigCreateSetupKey)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Create or use setup key");
    ui->infoFrame->setInfo(icons()->icon("key.png"), "One cosigner creates a new setup key and shares it with other "
                                                     "cosigners over a secure channel (e.g. end-to-end encrypted group chat.)");
}

void PageMultisigCreateSetupKey::initializePage() {
    // We may need to return to the wizard if it is closed before the multisig setup completes
    // Set a cache attribute to indicate that the setup is in progress
    m_fields->wallet->setCacheAttribute("feather.multisig_setup", "started");

    m_fields->mode = WizardMode::CreateMultisig;
}


int PageMultisigCreateSetupKey::nextId() const {
    if (ui->radio_createSetupKey->isChecked()) {
        // We are creating a new setup key
        return WalletWizard::Page_MultisigParticipants;
    }

    // We already have a setup key
    return WalletWizard::Page_MultisigEnterSetupKey;
}

bool PageMultisigCreateSetupKey::validatePage() {
    // Prevent double click on the previous page from going to the next page
    if (!ui->radio_haveSetupKey->isChecked() && !ui->radio_createSetupKey->isChecked()) {
        return false;
    }

    m_fields->multisigInitiator = ui->radio_createSetupKey->isChecked();
    return true;
}
