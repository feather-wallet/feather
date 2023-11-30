// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TickersConfigDialog.h"
#include "ui_TickersConfigDialog.h"

#include "utils/config.h"

#include "TickersConfigAddDialog.h"

TickersConfigDialog::TickersConfigDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::TickersConfigDialog)
{
    ui->setupUi(this);

    QStringList tickers = conf()->get(Config::tickers).toStringList();
    ui->tickerList->addItems(tickers);

    ui->check_showFiatBalance->setChecked(conf()->get(Config::tickersShowFiatBalance).toBool());
    connect(ui->check_showFiatBalance, &QCheckBox::toggled, [this](bool toggled) {
        conf()->set(Config::tickersShowFiatBalance, toggled);
    });

    connect(ui->btn_addTicker, &QPushButton::clicked, this, &TickersConfigDialog::addTicker);
    connect(ui->btn_removeTicker, &QPushButton::clicked, this, &TickersConfigDialog::removeTicker);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TickersConfigDialog::saveConfig);

    this->adjustSize();
}

void TickersConfigDialog::addTicker() {
    auto dialog = new TickersConfigAddDialog{this};
    switch (dialog->exec()) {
        case Accepted: {
            ui->tickerList->addItem(dialog->getTicker());
        }
        default:
            return;
    }
}

void TickersConfigDialog::removeTicker() {
    int currentRow = ui->tickerList->currentRow();
    if (currentRow < 0) {
        return;
    }

    ui->tickerList->takeItem(currentRow);
}

void TickersConfigDialog::saveConfig() {
    QStringList tickers;
    for (int i = 0; i < ui->tickerList->count(); i++) {
        tickers << ui->tickerList->item(i)->text();
    }
    conf()->set(Config::tickers, tickers);
}

TickersConfigDialog::~TickersConfigDialog() = default;