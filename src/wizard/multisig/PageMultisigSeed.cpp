// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSeed.h"
#include "ui_PageMultisigSeed.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"

#include "libwalletqt/Wallet.h"

PageMultisigSeed::PageMultisigSeed(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSeed)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Multisig seed");
//    this->setCommitPage(true);

    ui->infoFrame->setInfo(icons()->icon("seed.png"), "Store your multisig seed to a safe location.");

    connect(ui->btn_copy, &QPushButton::clicked, this, &PageMultisigSeed::copySeed);
}

void PageMultisigSeed::copySeed() {
    Utils::copyToClipboard(m_fields->wallet->getMultisigSeed());
}

void PageMultisigSeed::initializePage() {
    ui->seed->setPlainText(m_fields->wallet->getMultisigSeed());
}

int PageMultisigSeed::nextId() const {
    return WalletWizard::Page_MultisigMMSRecoveryInfo;
}

bool PageMultisigSeed::validatePage() {
    return true;
}

bool PageMultisigSeed::isComplete() const {
    return true;
}
