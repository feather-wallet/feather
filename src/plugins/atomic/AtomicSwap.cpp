//
// Created by dev on 6/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicSwap.h" resolved

#include "AtomicSwap.h"

#include <utility>
#include "ui_AtomicSwap.h"
#include "AtomicWidget.h"


AtomicSwap::AtomicSwap(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicSwap), fundDialog( new AtomicFundDialog(this)) {
    ui->setupUi(this);
    //ui->debug_log->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    QPixmap pixmapTarget = QPixmap(":/assets/images/hint-icon.png");
    int size=20;
    pixmapTarget = pixmapTarget.scaled(size-5, size-5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->btc_hint->setPixmap(pixmapTarget);
    ui->btc_hint->setToolTip("Alice is expected to send monero lock after one btc confirmation,\nswap is cancelable after 72 btc confirmations,\nyou will lose your funds if you don't refund before 144 confirmations");
    this->setContentsMargins(3,3,3,3);
    this->adjustSize();
    connect(ui->btn_cancel, &QPushButton::clicked, this, &AtomicSwap::cancel);
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
    btc_confs = confs;
    ui->label_btc_cons->setText(QString::number(confs));
    this->update();
}

void AtomicSwap::setTitle(QString title) {
    this->setWindowTitle(title);
    this->update();
}

void AtomicSwap::setSwap(QString swapId){
    id = std::move(swapId);
}

void AtomicSwap::cancel(){

}
