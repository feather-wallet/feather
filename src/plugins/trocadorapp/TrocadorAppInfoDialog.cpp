// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TrocadorAppInfoDialog.h"
#include "ui_TrocadorAppInfoDialog.h"

#include "utils/config.h"
#include "utils/Utils.h"

TrocadorAppInfoDialog::TrocadorAppInfoDialog(QWidget *parent, TrocadorAppModel *model, int row)
        : WindowModalDialog(parent)
        , ui(new Ui::TrocadorAppInfoDialog)
        , m_model(model)
        , m_row(row)
{
    ui->setupUi(this);

    setLabelText(ui->label_exchange, TrocadorAppModel::Exchange);
    setLabelText(ui->label_insurance, TrocadorAppModel::Insurance);
    setLabelText(ui->label_spread, TrocadorAppModel::Spread);
    setLabelText(ui->label_kycrating, TrocadorAppModel::KYCRating);

    connect(ui->btn_goToOffer, &QPushButton::clicked, this, &TrocadorAppInfoDialog::onGoToOffer);
}

void TrocadorAppInfoDialog::setLabelText(QLabel *label, TrocadorAppModel::Column column) {
    QString data = m_model->data(m_model->index(m_row, column)).toString();
    label->setText(data);
}

void TrocadorAppInfoDialog::onGoToOffer() {
    QString tradeId = m_model->getTradeId();
    QString frontend = config()->get(Config::trocadorAppFrontend).toString();
    QString offerUrl = QString("%1/exchange/%2").arg(frontend, tradeId);
    Utils::externalLinkWarning(this, offerUrl);
}

TrocadorAppInfoDialog::~TrocadorAppInfoDialog() = default;