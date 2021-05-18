// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "passworddialog.h"
#include "ui_passworddialog.h"

PasswordDialog::PasswordDialog(const QString &walletName, bool incorrectPassword, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);

    ui->label_wallet->setText(QString("Please enter password for wallet: %1").arg(walletName));
    ui->label_incorrectPassword->setVisible(incorrectPassword);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        password = ui->line_password->text();
    });

    this->adjustSize();
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}
