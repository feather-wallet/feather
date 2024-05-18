// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicConfigDialog.h"
#include "ui_AtomicConfigDialog.h"

#include "utils/AppData.h"
#include "utils/config.h"

AtomicConfigDialog::AtomicConfigDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AtomicConfigDialog)
{
    ui->setupUi(this);

    this->fillListWidgets();

    connect(ui->btn_selectAll, &QPushButton::clicked, this, &AtomicConfigDialog::selectAll);
    connect(ui->btn_deselectAll, &QPushButton::clicked, this, &AtomicConfigDialog::deselectAll);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        conf()->set(Config::fiatSymbols, this->checkedFiat());
        conf()->set(Config::cryptoSymbols, this->checkedCrypto());
        this->accept();
    });

    this->adjustSize();
}

QStringList AtomicConfigDialog::checkedFiat() {
    return this->getChecked(ui->list_fiat);
}

QStringList AtomicConfigDialog::checkedCrypto() {
    return this->getChecked(ui->list_crypto);
}

void AtomicConfigDialog::selectAll() {
    this->setCheckState(this->getVisibleListWidget(), Qt::Checked);
}

void AtomicConfigDialog::deselectAll() {
    this->setCheckState(this->getVisibleListWidget(), Qt::Unchecked);
}

void AtomicConfigDialog::setCheckState(QListWidget *widget, Qt::CheckState checkState) {
    QListWidgetItem *item;
    for (int i=0; i < widget->count(); i++) {
        item = widget->item(i);
        item->setCheckState(checkState);
    }
}

QStringList AtomicConfigDialog::getChecked(QListWidget *widget) {
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

QListWidget* AtomicConfigDialog::getVisibleListWidget() {
    if (ui->tabWidget->currentIndex() == 0) {
        return ui->list_fiat;
    } else {
        return ui->list_crypto;
    }
}

void AtomicConfigDialog::fillListWidgets() {
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

AtomicConfigDialog::~AtomicConfigDialog() = default;