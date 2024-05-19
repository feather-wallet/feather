// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicConfigDialog.h"
#include "ui_AtomicConfigDialog.h"

#include "utils/AppData.h"
#include "utils/config.h"
#include "utils/Networking.h"

AtomicConfigDialog::AtomicConfigDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AtomicConfigDialog)
{
    ui->setupUi(this);

    this->fillListWidgets();

    connect(ui->btn_autoInstall,&QPushButton::clicked, this, &AtomicConfigDialog::downloadBinary);
    connect(ui->btn_selectFile,&QPushButton::clicked, this, &AtomicConfigDialog::selectBinary);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        this->accept();
    });

    this->adjustSize();
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



void AtomicConfigDialog::fillListWidgets() {
    QStringList cryptoCurrencies = appData()->prices.markets.keys();
    QStringList fiatCurrencies = appData()->prices.rates.keys();

    QStringList checkedCryptoCurrencies = conf()->get(Config::cryptoSymbols).toStringList();
    QStringList checkedFiatCurrencies = conf()->get(Config::fiatSymbols).toStringList();


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

}

void AtomicConfigDialog::downloadBinary() {

};

void AtomicConfigDialog::selectBinary() {

};
AtomicConfigDialog::~AtomicConfigDialog() = default;