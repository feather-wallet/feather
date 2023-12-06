// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "AddressCheckerIndexDialog.h"
#include "ui_AddressCheckerIndexDialog.h"

#include "utils/Utils.h"
#include "components.h"
#include "dialog/QrCodeDialog.h"

AddressCheckerIndexDialog::AddressCheckerIndexDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AddressCheckerIndexDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_showQRcode, &QPushButton::clicked, [this]{
        QString address = this->address();
        QrCode qr(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH);
        QrCodeDialog dialog{this, &qr, "Address"};
        dialog.exec();
    });

    connect(ui->line_index, &QLineEdit::textChanged, [this]{
        ui->address->setText(this->address());
    });

    auto indexValidator = new U32Validator(this);
    ui->line_index->setValidator(indexValidator);
    ui->line_index->setText("0");

    this->adjustSize();
}

uint32_t AddressCheckerIndexDialog::index() {
    return ui->line_index->text().toUInt();
}

QString AddressCheckerIndexDialog::address() {
    return m_wallet->address(m_wallet->currentSubaddressAccount(), this->index());
}

AddressCheckerIndexDialog::~AddressCheckerIndexDialog() = default;