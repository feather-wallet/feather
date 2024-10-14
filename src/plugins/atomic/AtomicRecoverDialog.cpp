// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicRecoverDialog.h" resolved

#include "AtomicRecoverDialog.h"
#include "ui_AtomicRecoverDialog.h"
#include "config.h"
#include "AtomicSwap.h"
#include "Utils.h"
#include "constants.h"
#include <QStandardItemModel>

AtomicRecoverDialog::AtomicRecoverDialog(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicRecoverDialog), swapDialog(new AtomicSwap(this)) {
    ui->setupUi(this);
    auto model = new QStandardItemModel();
    ui->swap_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->swap_history->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->btn_refund_resume->setVisible(false);
    ui->swap_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->swap_history->verticalHeader()->setVisible(false);
    // Makes it easy to see if button is in refund or resume mode
    ui->btn_refund_resume->setProperty("Refund",0);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->raise();
    model->setHorizontalHeaderItem(0, new QStandardItem("Swap-Id"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Timestamp swap started"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Status"));

    QList<QStandardItem*> rowData;
    qDebug() << conf()->get(Config::pendingSwap);
    QStringList data = conf()->get(Config::pendingSwap).toStringList();
    qDebug() << data;
    for(int i=0; i< data.size(); i++){
            QStringList entry = data[i].split(":");
            qDebug() << "Swap-id - " + entry[0];
            QString id = entry[0];
            QDateTime timestamp = QDateTime::fromString(entry[1],"dd.MM.yyyy.hh.mm.ss");
            qint64 difference = timestamp.secsTo(QDateTime::currentDateTime());

            if (difference < 86400) { // 86400 is number of seconds in a day (if a swap is older it is punished)
                rowData.clear();
                rowData << new QStandardItem(id);
                rowData << new QStandardItem(timestamp.toString("MM-dd-yyyy hh:mm"));
                if (difference > 43200){ // 43200 is number of seconds in 12 hours
                    rowData << new QStandardItem("Refundable");
                } else
                    rowData << new QStandardItem("Recoverable/Pending Refund Timelock");
                model->appendRow(rowData);
            } else {
                data.remove(i);
            }
        }
    ui->swap_history->setModel(model);
    conf()->set(Config::pendingSwap,data);
    connect(ui->swap_history, &QAbstractItemView::clicked, this, &AtomicRecoverDialog::updateBtn);
    connect(ui->btn_refund_resume, &QPushButton::clicked, this, [this]{
        QStringList arguments;
        if (constants::networkType==NetworkType::STAGENET) {
            arguments << "--testnet";
        }
        arguments << "-j";
        arguments << "--debug";
        arguments << "-d";
        arguments << Config::defaultConfigDir().absolutePath();
        if (ui->btn_refund_resume->property("Refund").toBool()){
            arguments << "cancel-and-refund";
        } else {
            arguments << "resume";
        }
        arguments << "--swap-id";
        auto row = ui->swap_history->selectionModel()->selectedRows().at(0);
        arguments << row.sibling(row.row(),0).data().toString();
        if(conf()->get(Config::proxy).toInt() == Config::Proxy::Tor) {
            arguments << "--tor-socks5-port";
            arguments << conf()->get(Config::socks5Port).toString();
        }
        swapDialog->runSwap(arguments);
    });
}

void AtomicRecoverDialog::updateBtn(const QModelIndex &index){
    auto row = index.sibling(index.row(),2).data().toString();
    if (row.startsWith("Ref")){
        ui->btn_refund_resume->setText("Cancel And Refund");
        ui->btn_refund_resume->setProperty("Refund",1);
    } else {
        ui->btn_refund_resume->setText("Attempt to Resume");
        ui->btn_refund_resume->setProperty("Refund",0);
    }
    ui->btn_refund_resume->setVisible(true);

}

bool AtomicRecoverDialog::historyEmpty(){
    return conf()->get(Config::pendingSwap).value<QVariantList>().isEmpty();
}


void AtomicRecoverDialog::appendHistory(QString entry){
    auto current = conf()->get(Config::pendingSwap).value<QVariantList>();
    current.append(entry);
    conf()->set(Config::pendingSwap, current);
}
AtomicRecoverDialog::~AtomicRecoverDialog() {
    delete ui;
}
