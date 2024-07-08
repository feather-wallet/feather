//
// Created by dev on 6/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicSwap.h" resolved

#include "AtomicSwap.h"
#include "ui_AtomicSwap.h"
#include "AtomicWidget.h"


AtomicSwap::AtomicSwap(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicSwap) {
    ui->setupUi(this);
    //ui->debug_log->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    this->setContentsMargins(3,3,3,3);
    this->adjustSize();
}


AtomicSwap::~AtomicSwap() {
    delete ui;
    emit cleanProcs();
}
void AtomicSwap::logLine(QString line){
    //ui->debug_log->setText(ui->debug_log->toPlainText().append(QTime::currentTime().toString() + ":" + line));

    this->update();
}
void AtomicSwap::updateStatus(QString status){
    ui->label_status->setText(status);
    this->update();
}

void AtomicSwap::updateXMRConf(int confs) {
    ui->label_xmr_cons->setText(QString::number(confs));
    this->update();
}

void AtomicSwap::updateBTCConf(int confs) {
    ui->label_btc_cons->setText(QString::number(confs));
    this->update();
}

void AtomicSwap::setTitle(QString title) {
    this->setWindowTitle(title);
    this->update();
}

