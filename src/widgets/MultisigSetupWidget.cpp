// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "MultisigSetupWidget.h"
#include "ui_MultisigSetupWidget.h"

#include "ringct/rctOps.h"

MultisigSetupWidget::MultisigSetupWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::MultisigSetupWidget)
{
    ui->setupUi(this);

//    char setupKey[35];
//
//    auto key = rct::rct2sk(rct::skGen()
}


MultisigSetupWidget::~MultisigSetupWidget() = default;
