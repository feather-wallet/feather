// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "CalcConfigDialog.h"
#include "ui_CalcConfigDialog.h"

#include "utils/AppData.h"
#include "utils/config.h"

CalcConfigDialog::CalcConfigDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::CalcConfigDialog)
{
    ui->setupUi(this);

    this->fillListWidgets();

    connect(ui->btn_selectAll, &QPushButton::clicked, this, &CalcConfigDialog::selectAll);
    connect(ui->btn_deselectAll, &QPushButton::clicked, this, &CalcConfigDialog::deselectAll);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        conf()->set(Config::fiatSymbols, this->checkedFiat());
        conf()->set(Config::cryptoSymbols, this->checkedCrypto());
        this->accept();
    });

    this->adjustSize();
}

QStringList CalcConfigDialog::checkedFiat() {
    return this->getChecked(ui->list_fiat);
}

QStringList CalcConfigDialog::checkedCrypto() {
    return this->getChecked(ui->list_crypto);
}

void CalcConfigDialog::selectAll() {
    this->setCheckState(this->getVisibleListWidget(), Qt::Checked);
}

void CalcConfigDialog::deselectAll() {
    this->setCheckState(this->getVisibleListWidget(), Qt::Unchecked);
}

void CalcConfigDialog::setCheckState(QListWidget *widget, Qt::CheckState checkState) {
    QListWidgetItem *item;
    for (int i=0; i < widget->count(); i++) {
        item = widget->item(i);
        item->setCheckState(checkState);
    }
}

QStringList CalcConfigDialog::getChecked(QListWidget *widget) {
    QStringList checked;
    QListWidgetItem *item;
    for (int i=0; i < widget->count(); i++) {
        item = widget->item(i);
        if (item->checkState() == Qt::Checked) {
            checked.append(item->text());
        }
    }
    return checked;
}

QListWidget* CalcConfigDialog::getVisibleListWidget() {
    if (ui->tabWidget->currentIndex() == 0) {
        return ui->list_fiat;
    } else {
        return ui->list_crypto;
    }
}

void CalcConfigDialog::fillListWidgets() {
    QStringList cryptoCurrencies = appData()->prices.markets.keys();
    QStringList fiatCurrencies = appData()->prices.rates.keys();

    QStringList checkedCryptoCurrencies = conf()->get(Config::cryptoSymbols).toStringList();
    QStringList checkedFiatCurrencies = conf()->get(Config::fiatSymbols).toStringList();

    ui->list_crypto->addItems(cryptoCurrencies);
    ui->list_fiat->addItems(fiatCurrencies);

    auto setChecked = [](QListWidget *widget, const QStringList &checked){
        QListWidgetItem *item;
        for (int i=0; i < widget->count(); i++) {
            item = widget->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (checked.contains(item->text())) {
                item->setCheckState(Qt::Checked);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    };

    setChecked(ui->list_crypto, checkedCryptoCurrencies);
    setChecked(ui->list_fiat, checkedFiatCurrencies);
}

CalcConfigDialog::~CalcConfigDialog() = default;