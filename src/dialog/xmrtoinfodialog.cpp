// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "xmrtoinfodialog.h"
#include "ui_xmrtoinfodialog.h"

#include <QFont>

XmrToInfoDialog::XmrToInfoDialog(XmrToOrder *oInfo, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::XmrToInfoDialog)
        , m_oInfo(oInfo)
{
    ui->setupUi(this);

    ui->status->setText(XmrTo::stateMap[(OrderState) oInfo->state]);
    ui->xmrto_id->setText(!oInfo->uuid.isEmpty() ? oInfo->uuid : "");

    ui->error_code->setText(!oInfo->errorCode.isEmpty() ? oInfo->errorCode : "");
    ui->error_msg->setText(!oInfo->errorMsg.isEmpty() ? oInfo->errorMsg : "");

    ui->xmr_amount->setText(QString::number(oInfo->incoming_amount_total));
    ui->btc_amount->setText(QString::number(oInfo->btc_amount));
    ui->rate->setText(oInfo->incoming_price_btc > 0 ? QString::number(oInfo->incoming_price_btc) : "");

    ui->xmr_txid->setText(oInfo->xmr_txid);
    ui->xmr_address->setText(oInfo->receiving_subaddress);

    ui->btc_txid->setText(oInfo->btc_txid);
    ui->btc_address->setText(oInfo->btc_dest_address);

    this->adjustSize();
}

XmrToInfoDialog::~XmrToInfoDialog() {
    delete ui;
}
