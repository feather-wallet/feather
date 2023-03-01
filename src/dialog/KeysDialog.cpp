// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "KeysDialog.h"
#include "ui_KeysDialog.h"

KeysDialog::KeysDialog(Wallet *wallet, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::KeysDialog)
{
    ui->setupUi(this);

    QString unavailable = "Unavailable: Key is stored on hardware device";

    ui->label_restoreHeight->setText(QString::number(wallet->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(wallet->address(0, 0));
    ui->label_secretSpendKey->setText(wallet->isHwBacked() ? unavailable : wallet->getSecretSpendKey());
    ui->label_secretViewKey->setText(wallet->getSecretViewKey());
    ui->label_publicSpendKey->setText(wallet->getPublicSpendKey());
    ui->label_publicViewKey->setText(wallet->getPublicViewKey());

    this->adjustSize();
}

KeysDialog::~KeysDialog() = default;
