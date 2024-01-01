// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "LocalMoneroInfoDialog.h"
#include "ui_LocalMoneroInfoDialog.h"

#include "utils/config.h"
#include "utils/Utils.h"

LocalMoneroInfoDialog::LocalMoneroInfoDialog(QWidget *parent, LocalMoneroModel *model, int row)
        : WindowModalDialog(parent)
        , ui(new Ui::LocalMoneroInfoDialog)
        , m_model(model)
        , m_row(row)
{
    ui->setupUi(this);

    setLabelText(ui->label_price, LocalMoneroModel::PriceXMR);
    setLabelText(ui->label_seller, LocalMoneroModel::Seller);
    setLabelText(ui->label_paymentMethod, LocalMoneroModel::PaymentMethod);
    setLabelText(ui->label_paymentDetail, LocalMoneroModel::PaymentMethodDetail);
    setLabelText(ui->label_tradeLimits, LocalMoneroModel::Limits);

    QJsonObject offerData = model->getOffer(row);
    QString details = offerData["data"].toObject()["msg"].toString();
    details.remove("*");

    if (details.isEmpty()) {
        details = "No details.";
    }

    ui->info->setPlainText(details);

    connect(ui->btn_goToOffer, &QPushButton::clicked, this, &LocalMoneroInfoDialog::onGoToOffer);
}

void LocalMoneroInfoDialog::setLabelText(QLabel *label, LocalMoneroModel::Column column) {
    QString data = m_model->data(m_model->index(m_row, column)).toString();
    label->setText(data);
}

void LocalMoneroInfoDialog::onGoToOffer() {
    QJsonObject offerData = m_model->getOffer(m_row);
    QString frontend = conf()->get(Config::localMoneroFrontend).toString();
    QString offerUrl = QString("%1/ad/%2").arg(frontend, offerData["data"].toObject()["ad_id"].toString());
    Utils::externalLinkWarning(this, offerUrl);
}

LocalMoneroInfoDialog::~LocalMoneroInfoDialog() = default;