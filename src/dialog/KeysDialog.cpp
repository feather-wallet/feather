// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "KeysDialog.h"
#include "ui_KeysDialog.h"

KeysDialog::KeysDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::KeysDialog)
{
    ui->setupUi(this);

    QString unavailable = "Unavailable: Key is stored on hardware device";

    ui->label_restoreHeight->setText(QString::number(ctx->wallet->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(ctx->wallet->address(0, 0));
    ui->label_secretSpendKey->setText(ctx->wallet->isHwBacked() ? unavailable : ctx->wallet->getSecretSpendKey());
    ui->label_secretViewKey->setText(ctx->wallet->getSecretViewKey());
    ui->label_publicSpendKey->setText(ctx->wallet->getPublicSpendKey());
    ui->label_publicViewKey->setText(ctx->wallet->getPublicViewKey());

    this->adjustSize();
}

KeysDialog::~KeysDialog() = default;
