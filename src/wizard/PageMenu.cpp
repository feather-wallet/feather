// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PageMenu.h"
#include "ui_PageMenu.h"

#include <QTimer>

#include "WalletWizard.h"

PageMenu::PageMenu(WizardFields *fields, WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMenu)
        , m_walletKeysFilesModel(wallets)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open recent wallet");

    ui->label_version->setText(QString("Feather %1 — by featherwallet.org").arg(FEATHER_VERSION));

    QString settingsSkin = conf()->get(Config::skin).toString();
}

void PageMenu::initializePage() {
    if (m_walletKeysFilesModel->rowCount() > 0) {
        ui->radioOpen->setChecked(true);
    } else {
        ui->radioCreate->setChecked(true);
    }

    QTimer::singleShot(0, [this]{
        wizard()->button(QWizard::NextButton)->setFocus();
    });

    // Don't show setup wizard again
    conf()->set(Config::firstRun, false);
}

int PageMenu::nextId() const {
    if (ui->radioCreate->isChecked())
        return WalletWizard::Page_CreateWalletSeed;
    if (ui->radioOpen->isChecked())
        return WalletWizard::Page_OpenWallet;
    if (ui->radioSeed->isChecked())
        return WalletWizard::Page_WalletRestoreSeed;
    if (ui->radioViewOnly->isChecked())
        return WalletWizard::Page_WalletRestoreKeys;
    if (ui->radioCreateFromDevice->isChecked())
        return WalletWizard::Page_HardwareDevice;
    return 0;
}

bool PageMenu::validatePage() {
    m_fields->clearFields();

    if (ui->radioCreate->isChecked()) {
        m_fields->mode = WizardMode::CreateWallet;
        m_fields->modeText = "Create wallet";
    }
    if (ui->radioOpen->isChecked()) {
        m_fields->mode = WizardMode::OpenWallet;
        m_fields->modeText = "Open wallet";
    }
    if (ui->radioSeed->isChecked()) {
        m_fields->mode = WizardMode::RestoreFromSeed;
        m_fields->modeText = "Restore wallet";
    }
    if (ui->radioViewOnly->isChecked()) {
        m_fields->mode = WizardMode::RestoreFromKeys;
        m_fields->modeText = "Restore wallet";
    }
    if (ui->radioCreateFromDevice->isChecked()) {
        m_fields->mode = WizardMode::CreateWalletFromDevice;
        m_fields->modeText = "Create from hardware device";
    }

    return true;
}