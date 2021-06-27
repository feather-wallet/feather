// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PasswordDialog.h"
#include "ui_PasswordDialog.h"
#include "utils/Icons.h"

PasswordDialog::PasswordDialog(const QString &walletName, bool incorrectPassword, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);

    this->setWindowIcon(icons()->icon("appicons/64x64.png"));
    ui->label_wallet->setText(QString("Please enter password for wallet: %1").arg(walletName));
    ui->label_incorrectPassword->setVisible(incorrectPassword);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        password = ui->line_password->text();
    });

    this->adjustSize();
}

PasswordDialog::~PasswordDialog() = default;