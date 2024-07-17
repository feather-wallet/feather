// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigOwnAddress.h"
#include "ui_PageMultisigOwnAddress.h"

#include "utils/Icons.h"
#include "utils/Utils.h"

PageMultisigOwnAddress::PageMultisigOwnAddress(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigOwnAddress)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Your wallet address");

    ui->infoFrame->setInfo(icons()->icon("tab_addresses.png"), "Copy the address below and send it to all co-signers.\n\nThe address is used to encrypt messages.");

    connect(ui->btn_copyAddress, &QPushButton::clicked, [this]{
       Utils::copyToClipboard(m_fields->wallet->address(0, 0));
    });
}

void PageMultisigOwnAddress::initializePage() {
    ui->line_ownAddress->setPlainText(m_fields->wallet->address(0, 0));
}

int PageMultisigOwnAddress::nextId() const {
    return WalletWizard::Page_MultisigSignerInfo;
}
