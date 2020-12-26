// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "wizard/menu.h"
#include "wizard/walletwizard.h"
#include "ui_menu.h"

#include <QPushButton>
#include <QFileDialog>
#include <QDebug>

#include "libwalletqt/WalletManager.h"

MenuPage::MenuPage(AppContext *ctx, WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::MenuPage)
        , m_ctx(ctx)
        , m_walletKeysFilesModel(wallets)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open recent wallet");
}

void MenuPage::initializePage() {
    if (m_walletKeysFilesModel->rowCount() > 0) {
        ui->radioOpen->setChecked(true);
    } else {
        ui->radioCreate->setChecked(true);
    }
}

int MenuPage::nextId() const {
    if (ui->radioOpen->isChecked())
        return WalletWizard::Page_OpenWallet;
    if (ui->radioCreate->isChecked())
        return WalletWizard::Page_CreateWallet;
    if(ui->radioSeed->isChecked())
        return WalletWizard::Page_Restore;
    if(ui->radioViewOnly->isChecked())
        return WalletWizard::Page_ViewOnly;
    return 0;
}

bool MenuPage::validatePage() {
    // Check if file exists
    // Check if wallet has password
    // Check if wallet can be decrypted with entered password

    // TODO: Check if password is correct, otherwise show error message
    return true;
}