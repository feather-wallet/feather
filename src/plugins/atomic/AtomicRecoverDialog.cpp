//
// Created by dev on 7/29/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicRecoverDialog.h" resolved

#include "AtomicRecoverDialog.h"
#include "ui_AtomicRecoverDialog.h"
#include "History.h"
#include "config.h"
#include <QStandardItemModel>

AtomicRecoverDialog::AtomicRecoverDialog(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicRecoverDialog) {
    ui->setupUi(this);
    auto model = new QStandardItemModel();
    ui->swap_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->swap_history->setModel(model);
    model->setHorizontalHeaderItem(0, new QStandardItem("Swap-Id"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Timestamp swap started"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Status"));

    QList<QStandardItem*> rowData;
    auto data = Config::instance()->get(Config::pendingSwap).value<QVariantList>();
    for(int i=0; i< data.size(); i++){
            auto entry = data[i].value<HistoryEntry>();
            qint64 difference = entry.timestamp.secsTo(QDateTime::currentDateTime());

            if (difference < 86400) {
                rowData.clear();
                rowData << new QStandardItem(entry.id);
                rowData << new QStandardItem(entry.timestamp.toString("MM-dd-yyyy hh:mm"));
                if (difference > 43200){
                    rowData << new QStandardItem("Refundable");
                } else
                    rowData << new QStandardItem("Recoverable/Pending Refund Timelock");
                model->appendRow(rowData);
            } else {
                data.remove(i);
            }
        }
    Config::instance()->set(Config::pendingSwap,data);
}

bool AtomicRecoverDialog::historyEmpty(){
    return Config::instance()->get(Config::pendingSwap).value<QVariantList>().isEmpty();
}


void AtomicRecoverDialog::appendHistory(HistoryEntry entry){
    auto current = Config::instance()->get(Config::pendingSwap).value<QVariantList>();
    auto var = QVariant();
    var.setValue(entry);
    current.append(var);
    Config::instance()->set(Config::pendingSwap, current);
}
AtomicRecoverDialog::~AtomicRecoverDialog() {
    delete ui;
}
