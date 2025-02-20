// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TickersConfigAddDialog.h"
#include "ui_TickersConfigAddDialog.h"

#include "utils/AppData.h"
#include "utils/config.h"

TickersConfigAddDialog::TickersConfigAddDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::TickersConfigAddDialog)
{
    ui->setupUi(this);

    QStringList cryptoCurrencies = appData()->prices.markets.keys();
    QStringList fiatCurrencies = appData()->prices.rates.keys();

    QStringList allCurrencies;
    allCurrencies << cryptoCurrencies << fiatCurrencies;

    ui->comboTicker->addItems(cryptoCurrencies);
    ui->comboRatio1->addItems(cryptoCurrencies);
    ui->comboRatio2->addItems(cryptoCurrencies);

    connect(ui->combo_type, &QComboBox::currentIndexChanged, [this](int index) {
        ui->stackedWidget->setCurrentIndex(index);
    });

    this->adjustSize();
}

QString TickersConfigAddDialog::getTicker() {
    int type = ui->combo_type->currentIndex();

    if (type == TickersType::TICKER) {
        return ui->comboTicker->currentText();
    }

    if (type == TickersType::RATIO) {
        return QString("%1/%2").arg(ui->comboRatio1->currentText(), ui->comboRatio2->currentText());
    }

    return {};
}

TickersConfigAddDialog::~TickersConfigAddDialog() = default;