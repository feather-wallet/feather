// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "keysdialog.h"
#include "ui_keysdialog.h"

KeysDialog::KeysDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::KeysDialog)
{
    ui->setupUi(this);

    ui->label_restoreHeight->setText(QString::number(ctx->currentWallet->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(ctx->currentWallet->address(0, 0));
    ui->label_secretSpendKey->setText(ctx->currentWallet->getSecretSpendKey());
    ui->label_secretViewKey->setText(ctx->currentWallet->getSecretViewKey());
    ui->label_publicSpendKey->setText(ctx->currentWallet->getPublicSpendKey());
    ui->label_publicViewKey->setText(ctx->currentWallet->getPublicViewKey());

    this->adjustSize();
}

KeysDialog::~KeysDialog()
{
    delete ui;
}
