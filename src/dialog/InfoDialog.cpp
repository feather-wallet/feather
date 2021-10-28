// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "InfoDialog.h"
#include "ui_InfoDialog.h"

InfoDialog::InfoDialog(QWidget *parent, const QString &title, const QString &infoData)
        : WindowModalDialog(parent)
        , ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(title);
    ui->info->setPlainText(infoData);
}

InfoDialog::~InfoDialog() = default;