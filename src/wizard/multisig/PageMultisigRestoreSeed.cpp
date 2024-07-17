// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigRestoreSeed.h"
#include "ui_PageMultisigRestoreSeed.h"

#include <QCheckBox>

#include "wizard/WalletWizard.h"

PageMultisigRestoreSeed::PageMultisigRestoreSeed(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigRestoreSeed)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Restore multisig wallet");

    connect(ui->btn_options, &QPushButton::clicked, this, &PageMultisigRestoreSeed::onOptionsClicked);
}

void PageMultisigRestoreSeed::initializePage() {
    ui->multisigSeed->setPlainText("");
}

int PageMultisigRestoreSeed::nextId() const {
    return WalletWizard::Page_MultisigRestoreMMSRecoveryInfo;
}

bool PageMultisigRestoreSeed::validatePage() {
    m_fields->multisigSeed = ui->multisigSeed->toPlainText();
    return true;
}

void PageMultisigRestoreSeed::onOptionsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Options");

    QVBoxLayout layout;
    QCheckBox check_subaddressLookahead("Set subaddress lookahead");
    check_subaddressLookahead.setChecked(m_fields->showSetSubaddressLookaheadPage);

    layout.addWidget(&check_subaddressLookahead);
    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();

    m_fields->showSetSubaddressLookaheadPage = check_subaddressLookahead.isChecked();
}