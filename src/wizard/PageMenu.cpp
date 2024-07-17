// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "WalletWizard.h"
#include "PageMenu.h"
#include "ui_PageMenu.h"

#include <QFileDialog>

#include "config-feather.h"

PageMenu::PageMenu(WizardFields *fields, WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMenu)
        , m_walletKeysFilesModel(wallets)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open recent wallet");

    ui->label_version->setText(QString("Feather %1 — by dsc & tobtoht").arg(FEATHER_VERSION));

    QString settingsSkin = conf()->get(Config::skin).toString();

//    connect(ui->btn_create, &QPushButton::clicked, [this]{
//        m_fields->mode = WizardMode::CreateWallet;
//        m_fields->modeText = "Create wallet";
//        m_nextPage = WalletWizard::Page_Recover;
//        wizard()->button(QWizard::NextButton)->click();
//    });
//
//    connect(ui->btn_open, &QPushButton::clicked, [this]{
//        m_fields->mode = WizardMode::OpenWallet;
//        m_fields->modeText = "Open wallet";
//        m_nextPage = WalletWizard::Page_OpenWallet;
//        wizard()->button(QWizard::NextButton)->click();
//    });
//
//    connect(ui->btn_restore, &QPushButton::clicked, [this]{
////        m_fields->mode = WizardMode::RecoverWallet;
//        m_fields->modeText = "Restore wallet";
//        m_nextPage = WalletWizard::Page_Recover;
//        wizard()->button(QWizard::NextButton)->click();
//    });
}

void PageMenu::initializePage() {
    if (m_walletKeysFilesModel->rowCount() > 0) {
        ui->radio_open->setChecked(true);
    } else {
        ui->radio_create->setChecked(true);
    }

    // Don't show setup wizard again
    conf()->set(Config::firstRun, false);
}

int PageMenu::nextId() const {
    if (ui->radio_create->isChecked())
        return WalletWizard::Page_CreateWalletSeed;
    if (ui->radio_createMultisig->isChecked())
        return WalletWizard::Page_MultisigExperimentalWarning;
    if (ui->radio_open->isChecked())
        return WalletWizard::Page_OpenWallet;
    if (ui->radio_restoreSeed->isChecked())
        return WalletWizard::Page_WalletRestoreSeed;
    if (ui->radio_restoreKeys->isChecked())
        return WalletWizard::Page_WalletRestoreKeys;
    if (ui->radio_restoreMultisig->isChecked())
        return WalletWizard::Page_MultisigRestoreSeed;
    if (ui->radio_restoreHardware->isChecked())
        return WalletWizard::Page_HardwareDevice;
    return 0;
}

bool PageMenu::validatePage() {
    m_fields->clearFields();

    if (ui->radio_create->isChecked()) {
        m_fields->mode = WizardMode::CreateWallet;
        m_fields->modeText = "Create wallet";
    }
    if (ui->radio_createMultisig->isChecked()) {
        m_fields->mode = WizardMode::CreateMultisig;
        m_fields->modeText = "Create multisig wallet";
    }
    if (ui->radio_open->isChecked()) {
        m_fields->mode = WizardMode::OpenWallet;
        m_fields->modeText = "Open wallet";
    }
    if (ui->radio_restoreSeed->isChecked()) {
        m_fields->mode = WizardMode::RestoreFromSeed;
        m_fields->modeText = "Restore wallet";
    }
    if (ui->radio_restoreKeys->isChecked()) {
        m_fields->mode = WizardMode::RestoreFromKeys;
        m_fields->modeText = "Restore wallet";
    }
    if (ui->radio_restoreMultisig->isChecked()) {
        m_fields->mode = WizardMode::RestoreMultisig;
        m_fields->modeText = "Restore multisig wallet";
    }
    if (ui->radio_restoreHardware->isChecked()) {
        m_fields->mode = WizardMode::CreateWalletFromDevice;
        m_fields->modeText = "Create from hardware device";
    }

    return true;
}