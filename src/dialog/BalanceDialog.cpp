// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "BalanceDialog.h"
#include "ui_BalanceDialog.h"

#include "libwalletqt/WalletManager.h"
#include "model/ModelUtils.h"

BalanceDialog::BalanceDialog(QWidget *parent, Wallet *wallet)
        : WindowModalDialog(parent)
        , ui(new Ui::BalanceDialog)
{
    ui->setupUi(this);

    ui->label_unconfirmed_help->setHelpText("Outputs require 10 confirmations before they become spendable. "
                                            "This will take 20 minutes on average.");

    ui->label_unconfirmed->setText(WalletManager::displayAmount(wallet->balance() - wallet->unlockedBalance()));
    ui->label_unconfirmed->setFont(ModelUtils::getMonospaceFont());

    ui->label_spendable->setText(WalletManager::displayAmount(wallet->unlockedBalance()));
    ui->label_spendable->setFont(ModelUtils::getMonospaceFont());

    ui->label_total->setText(WalletManager::displayAmount(wallet->balance()));
    ui->label_total->setFont(ModelUtils::getMonospaceFont());

    this->adjustSize();
}

BalanceDialog::~BalanceDialog() = default;
