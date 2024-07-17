// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSignerInfo.h"
#include "ui_PageMultisigSignerInfo.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"
#include "libwalletqt/MultisigMessageStore.h"

PageMultisigSignerInfo::PageMultisigSignerInfo(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSignerInfo)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Enter signer info");
//    this->setCommitPage(true);

    ui->infoFrame->setInfo(icons()->icon("sign.png"), "Enter a label and the address for each cosigner.");
}

void PageMultisigSignerInfo::initializePage() {
    while (QLayoutItem* item = ui->signerLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_signerInfo.clear();

    for (int i = 1; i < m_fields->multisigSigners; i++) {
        auto *name = new QLineEdit(this);
        auto *address = new QLineEdit(this);

        m_signerInfo.push_back({name, address});
        ui->signerLayout->addRow(name, address);
    }
}

int PageMultisigSignerInfo::nextId() const {
    return WalletWizard::Page_MultisigSetupWallet;
}

bool PageMultisigSignerInfo::validatePage() {
    // TODO: error handling
    for (int i = 0; i < m_signerInfo.size(); i++) {
        const auto& info = m_signerInfo[i];
        m_fields->wallet->mmsStore()->setSigner(i+1, info.first->text(), info.second->text());
    }

    m_fields->wallet->mmsStore()->next();

    return true;
}

bool PageMultisigSignerInfo::isComplete() const {
    return true;
}

