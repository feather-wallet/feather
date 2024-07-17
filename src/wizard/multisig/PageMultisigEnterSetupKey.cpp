// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigEnterSetupKey.h"
#include "ui_PageMultisigEnterSetupKey.h"

#include "libwalletqt/MultisigMessageStore.h"
#include "utils/Icons.h"

PageMultisigEnterSetupKey::PageMultisigEnterSetupKey(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigEnterSetupKey)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Enter setup key");

    ui->infoFrame->setInfo(icons()->icon("key.png"), "The setup key should not be shared with outsiders and only used once per participant.");

    connect(ui->line_setupKey, &QLineEdit::textChanged, this, &PageMultisigEnterSetupKey::checkSetupKey);
    connect(ui->radio_yes, &QRadioButton::toggled, [this](bool toggled){
       completeChanged();
    });
}

void PageMultisigEnterSetupKey::checkSetupKey(const QString &setupKey) {
    ui->frame_invalid->hide();
    ui->frame_verify->hide();

    if (setupKey.isEmpty()) {
        return;
    }

    MultisigMessageStore::SetupKey key;
    bool keyValid = m_fields->wallet->mmsStore()->checkSetupKey(setupKey, key);

    if (!keyValid) {
      ui->frame_invalid->show();
      return;
    }

    m_fields->multisigThreshold = key.threshold;
    m_fields->multisigSigners = key.participants;
    m_fields->multisigService = key.service;
    m_fields->multisigAutomaticSetup = (key.mode == MultisigMessageStore::SetupMode::AUTOMATIC);
    m_fields->multisigSetupKey = setupKey;

    ui->frame_verify->show();
    ui->label_verify->setText(QString("You are setting up a %1-of-%2 multisig wallet. Is that correct?").arg(QString::number(key.threshold), QString::number(key.participants)));
}

void PageMultisigEnterSetupKey::initializePage() {
    ui->frame_invalid->hide();
    ui->frame_verify->hide();
    ui->line_setupKey->setText("");
}

int PageMultisigEnterSetupKey::nextId() const {
    return WalletWizard::Page_MultisigEnterChannel;
}

bool PageMultisigEnterSetupKey::validatePage() {
    m_fields->multisigSetupKey = ui->line_setupKey->text();
    return true;
}

bool PageMultisigEnterSetupKey::isComplete() const {
    if (ui->radio_yes->isChecked()) {
        return true;
    }

    return false;
}

