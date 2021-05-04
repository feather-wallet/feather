// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "keysdialog.h"
#include "ui_keysdialog.h"

KeysDialog::KeysDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::KeysDialog)
{
    ui->setupUi(this);

    auto w = ctx->currentWallet;
    QString unavailable = "Unavailable: Key is stored on hardware device";

    ui->label_restoreHeight->setText(QString::number(w->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(w->address(0, 0));
    ui->label_secretSpendKey->setText(w->isHwBacked() ? unavailable : w->getSecretSpendKey());
    ui->label_secretViewKey->setText(w->getSecretViewKey());
    ui->label_publicSpendKey->setText(w->getPublicSpendKey());
    ui->label_publicViewKey->setText(w->getPublicViewKey());

    this->adjustSize();
}

KeysDialog::~KeysDialog()
{
    delete ui;
}
