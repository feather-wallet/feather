// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "AddressCheckerIndexDialog.h"
#include "ui_AddressCheckerIndexDialog.h"

#include "utils/Utils.h"
#include "components.h"

AddressCheckerIndexDialog::AddressCheckerIndexDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AddressCheckerIndexDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_showAddress, &QPushButton::clicked, [this] {
        this->showAddress(ui->line_index->text().toUInt());
    });

    auto indexValidator = new U32Validator(this);
    ui->line_index->setValidator(indexValidator);
    ui->line_index->setText("0");

    this->showAddress(0);
    this->adjustSize();
}

void AddressCheckerIndexDialog::showAddress(uint32_t index) {
    ui->address->setText(m_wallet->address(m_wallet->currentSubaddressAccount(), index));
}

AddressCheckerIndexDialog::~AddressCheckerIndexDialog() = default;