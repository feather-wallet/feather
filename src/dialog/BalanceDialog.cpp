// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "BalanceDialog.h"
#include "ui_BalanceDialog.h"

#include "libwalletqt/WalletManager.h"
#include "utils/Utils.h"

BalanceDialog::BalanceDialog(QWidget *parent, Wallet *wallet)
        : WindowModalDialog(parent)
        , m_wallet(wallet)
        , ui(new Ui::BalanceDialog)
{
    ui->setupUi(this);

    connect(m_wallet, &Wallet::balanceUpdated, this, &BalanceDialog::updateBalance);

    ui->label_unconfirmed_help->setHelpText("Unconfirmed balance", "Outputs require 10 confirmations before they become spendable. "
                                            "This will take 20 minutes on average.", "balance");

    ui->label_unconfirmed->setFont(Utils::getMonospaceFont());
    ui->label_spendable->setFont(Utils::getMonospaceFont());
    ui->label_total->setFont(Utils::getMonospaceFont());

    this->updateBalance();

    this->adjustSize();
}

void BalanceDialog::updateBalance() {
    ui->label_unconfirmed->setText(WalletManager::displayAmount(m_wallet->balance() - m_wallet->unlockedBalance()));
    ui->label_spendable->setText(WalletManager::displayAmount(m_wallet->unlockedBalance()));
    ui->label_total->setText(WalletManager::displayAmount(m_wallet->balance()));
}

BalanceDialog::~BalanceDialog() = default;
