// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigVerifyAddress.h"
#include "ui_PageMultisigVerifyAddress.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"
#include "libwalletqt/MultisigMessageStore.h"

PageMultisigVerifyAddress::PageMultisigVerifyAddress(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigVerifyAddress)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Verify multisig address");
//    this->setCommitPage(true);

    ui->infoFrame->setInfo(icons()->icon("tab_addresses.png"), "Verify that all participants share the same multisig address.");

    connect(ui->check_verify, &QCheckBox::toggled, [this](bool checked){
       completeChanged();
    });

    connect(ui->btn_copyAddress, &QPushButton::clicked, [this] {
        Utils::copyToClipboard(m_address);
    });
}

void PageMultisigVerifyAddress::initializePage() {
    bool ok;
    QString reason;
    m_address = m_fields->wallet->getAddressSafe(0, 0, ok, reason);

    if (!ok) {
        ui->label_address->setText("");
        Utils::showError(this, "Unable to get multisig address", reason);
        return;
    }

    ui->label_address->setText(Utils::chunkAddress(m_address));
    ui->label_address->setFont(Utils::getMonospaceFont());
}

int PageMultisigVerifyAddress::nextId() const {
    return WalletWizard::Page_MultisigSeed;
}

bool PageMultisigVerifyAddress::validatePage() {
    return true;
}

bool PageMultisigVerifyAddress::isComplete() const {
    if (!ui->check_verify->isChecked()) {
        return false;
    }

    return true;
}

