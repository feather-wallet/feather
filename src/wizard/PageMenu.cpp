// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WalletWizard.h"
#include "PageMenu.h"
#include "ui_PageMenu.h"

#include <QFileDialog>

PageMenu::PageMenu(WizardFields *fields, WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMenu)
        , m_walletKeysFilesModel(wallets)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open recent wallet");

#if defined(Q_OS_MAC)
    ui->check_darkMode->setVisible(false);
#endif

    QString settingsSkin = config()->get(Config::skin).toString();
    ui->check_darkMode->setChecked(settingsSkin == "QDarkStyle");

    connect(ui->check_darkMode, &QCheckBox::toggled, this, &PageMenu::enableDarkMode);
}

void PageMenu::initializePage() {
    if (m_walletKeysFilesModel->rowCount() > 0) {
        ui->radioOpen->setChecked(true);
    } else {
        ui->radioCreate->setChecked(true);
    }
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