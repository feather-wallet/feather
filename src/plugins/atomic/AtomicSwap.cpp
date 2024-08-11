//
// Created by dev on 6/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicSwap.h" resolved

#include "AtomicSwap.h"

#include <QJsonParseError>
#include <qt6/QtWidgets/QMessageBox>
#include "ui_AtomicSwap.h"
#include "AtomicWidget.h"


AtomicSwap::AtomicSwap(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicSwap), fundDialog( new AtomicFundDialog(this)), procList(new QList<QSharedPointer<QProcess>>()) {
    ui->setupUi(this);
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

void AtomicSwap::runSwap(QStringList arguments){
    auto *swap = new QProcess();
    procList->append(QSharedPointer<QProcess>(swap));
    swap->setProcessChannelMode(QProcess::MergedChannels);
    swap->setReadChannel(QProcess::StandardOutput);
    connect(swap, &QProcess::readyRead,this, [this, swap] {
        while(swap->canReadLine()){
            QJsonParseError err;
            const QByteArray& rawline = swap->readLine();
            QJsonDocument line = QJsonDocument::fromJson(rawline, &err);
            qDebug() << rawline;
            bool check;
            if (line["fields"]["message"].toString().contains("Connected to Alice")){
                qDebug() << "Successfully connected";
                this->logLine(line["fields"].toString());
            } else if (!line["fields"]["deposit_address"].toString().isEmpty()){
                qDebug() << "Deposit to btc to segwit address";
                QString address = line["fields"]["deposit_address"].toString();
                fundDialog = new AtomicFundDialog(this,  "Deposit BTC to this address", address);
                //dialog->setModal(true);
                fundDialog->show();
            } else if (line["fields"]["message"].toString().startsWith("Received Bitcoin")){
                this->updateStatus(line["fields"]["new_balance"].toString().split(" ")[0] + " BTC received, starting swap");
                QVariant var;
                var.setValue(HistoryEntry {QDateTime::currentDateTime(),line["span"]["swap_id"].toString()});
                QVariantList past = conf()->get(Config::pendingSwap).toList();
                past.append(var);
                conf()->set(Config::pendingSwap,past);
                fundDialog->close();
                qDebug() << "Spawn atomic swap progress dialog";
                this->show();
            } else if ( QString confs = line["fields"]["seen_confirmations"].toString(); !confs.isEmpty()){
                qDebug() << "Updating xmrconfs " + confs;
                this->updateXMRConf(confs.toInt());
            } else if (QString message = line["fields"]["message"].toString(); QString::compare(message, "Bitcoin transaction status changed")==0){
                qDebug() << "Updating btconfs " + line["fields"]["new_status"].toString().split(" ")[2];
                QString status = line["fields"]["new_status"].toString();
                bool ok;
                auto confirmations = status.split(" ")[2].toInt(&ok, 10);
                if(ok) {
                    this->updateBTCConf(confirmations);
                } else {
                    this->updateStatus("Found txid " + line["fields"]["txid"].toString() + " in mempool");
                }
            } else if (QString message = line["fields"]["message"].toString(); message.startsWith("Swap completed")){
                QVariantList past = conf()->get(Config::pendingSwap).toList();
                past.removeLast();
                conf()->set(Config::pendingSwap, past);
                this->updateStatus("Swap has successfully completed you can close this window now");
            } else if (QString message = line["fields"]["message"].toString(); QString::compare(message,"Advancing state")==0){
                this->updateStatus("State of swap has advanced to " + line["fields"]["state"].toString());
            } else if (QString refund = line["fields"]["kind"].toString(); QString::compare(refund,"refund")==0){
                QString txid = line["fields"]["txid"].toString();
                QString id = line["span"]["swap_id"].toString();
                QVariantList past = conf()->get(Config::pendingSwap).toList();
                for(int i=0;i<past.length();i++){
                    if(QString::compare(past[i].value<HistoryEntry>().id,id)==0) {
                        past.remove(i);
                        break;
                    }
                }
                conf()->set(Config::pendingSwap, past);
                QMessageBox::information(this,"Cancel and Refund","Swap refunded succesfully with txid " + txid);
            } else if (QString message = line["fields"]["message"].toString(); QString::compare(message, "API call resulted in an error")==0){
                QString err = line["fields"]["err"].toString().split("\n")[0].split(":")[1];
                QMessageBox::warning(this, "Cancel and Refund", "Time lock hasn't expired yet so cancel failed. Try again in " + err + "blocks");
            } else if (QString message = line["fields"]["latest_version"].toString(); !message.isEmpty()){
                conf()->set(Config::swapVersion,message);
            }
        }
    });

    swap->start(conf()->get(Config::swapPath).toString(),arguments);
    qDebug() << "process started";
}
AtomicSwap::~AtomicSwap() {
    delete ui;
    for (const auto& proc : *procList){
        proc->kill();
    }
}
void AtomicSwap::logLine(QString line){
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


void AtomicSwap::cancel(){

}
