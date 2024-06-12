//
// Created by dev on 6/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicSwap.h" resolved

#include "AtomicSwap.h"
#include "ui_AtomicSwap.h"



AtomicSwap::AtomicSwap(QWidget *parent) :
        QDialog(parent), ui(new Ui::AtomicSwap) {
    ui->setupUi(this);
}


AtomicSwap::~AtomicSwap() {
    delete ui;
}
void AtomicSwap::logLine(QString line){
    ui->debug_log->setText(ui->debug_log->toPlainText().append(QTime::currentTime().toString() + ":" + line));
    this->update();
}
void AtomicSwap::updateStatus(QString status){
    ui->label_status->setText(status);
    this->update();
}