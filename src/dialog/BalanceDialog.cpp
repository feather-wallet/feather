// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "BalanceDialog.h"
#include "ui_BalanceDialog.h"

#include "libwalletqt/WalletManager.h"
#include "utils/Utils.h"
#include "utils/config.h"
#include "utils/AppData.h"
#include "constants.h"

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

    ui->label_unconfirmedFiat->setFont(Utils::getMonospaceFont());
    ui->label_spendableFiat->setFont(Utils::getMonospaceFont());
    ui->label_totalFiat->setFont(Utils::getMonospaceFont());

    if (conf()->get(Config::disableWebsocket).toBool()) {
        ui->label_unconfirmedFiat->hide();
        ui->label_spendableFiat->hide();
        ui->label_totalFiat->hide();
    }

    this->updateBalance();

    this->adjustSize();
}

void BalanceDialog::updateBalance() {
    quint64 unconfirmedBalance = m_wallet->balance() - m_wallet->unlockedBalance();

    ui->label_unconfirmed->setText(WalletManager::displayAmount(unconfirmedBalance));
    ui->label_spendable->setText(WalletManager::displayAmount(m_wallet->unlockedBalance()));
    ui->label_total->setText(WalletManager::displayAmount(m_wallet->balance()));

    if (!conf()->get(Config::disableWebsocket).toBool()) {
        ui->label_unconfirmedFiat->setText(appData()->prices.atomicUnitsToPreferredFiatString(unconfirmedBalance));
        ui->label_spendableFiat->setText(appData()->prices.atomicUnitsToPreferredFiatString(m_wallet->unlockedBalance()));
        ui->label_totalFiat->setText(appData()->prices.atomicUnitsToPreferredFiatString(m_wallet->balance()));
    }
}

BalanceDialog::~BalanceDialog() = default;
