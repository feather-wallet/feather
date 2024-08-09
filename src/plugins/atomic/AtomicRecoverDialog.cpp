//
// Created by dev on 7/29/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicRecoverDialog.h" resolved

#include "AtomicRecoverDialog.h"
#include "ui_AtomicRecoverDialog.h"
#include "History.h"
#include "config.h"
#include "AtomicSwap.h"
#include <QStandardItemModel>

AtomicRecoverDialog::AtomicRecoverDialog(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicRecoverDialog), swapDialog(new AtomicSwap(this)) {
    ui->setupUi(this);
    auto model = new QStandardItemModel();
    ui->swap_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->swap_history->setModel(model);
    ui->btn_refund_resume->setVisible(false);
    ui->swap_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->swap_history->verticalHeader()->setVisible(false);
    // Makes it easy to see if button is in refund or resume mode
    ui->btn_refund_resume->setProperty("Refund",0);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    model->setHorizontalHeaderItem(0, new QStandardItem("Swap-Id"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Timestamp swap started"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Status"));

    QList<QStandardItem*> rowData;
    auto data = conf()->get(Config::pendingSwap).value<QVariantList>();
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
    conf()->set(Config::pendingSwap,data);
    connect(ui->swap_history, &QAbstractItemView::clicked, this, &AtomicRecoverDialog::updateBtn);
    connect(ui->btn_refund_resume, &QPushButton::clicked, this, [this]{
        QStringList arguments;
        if (ui->btn_refund_resume->property("Refund").toBool()){
            arguments << "cancel-and-refund";
        } else {
            arguments << "resume";
        }
        arguments << "--swap-id";
        arguments << ui->swap_history->selectionModel()->selectedRows().at(0).sibling(0,1).data().toString();
        arguments << "--tor-socks5-port";
        arguments << conf()->get(Config::socks5Port).toString();
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


void AtomicRecoverDialog::appendHistory(HistoryEntry entry){
    auto current = conf()->get(Config::pendingSwap).value<QVariantList>();
    auto var = QVariant();
    var.setValue(entry);
    current.append(var);
    conf()->set(Config::pendingSwap, current);
}
AtomicRecoverDialog::~AtomicRecoverDialog() {
    delete ui;
}
