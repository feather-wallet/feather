//
// Created by dev on 6/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AtomicSwap.h" resolved

#include "AtomicSwap.h"

#include <QJsonParseError>
#include <QMessageBox>
#include "ui_AtomicSwap.h"
#include "AtomicWidget.h"
#include "constants.h"
#include "networktype.h"


AtomicSwap::AtomicSwap(QWidget *parent) :
        WindowModalDialog(parent), ui(new Ui::AtomicSwap), fundDialog( new AtomicFundDialog(this)), procList(new QList<QSharedPointer<QProcess>>()) {
    ui->setupUi(this);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    QPixmap pixmapTarget = QPixmap(":/assets/images/hint-icon.png");
    int size=20;
    pixmapTarget = pixmapTarget.scaled(size-5, size-5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->btc_hint->setPixmap(pixmapTarget);
    ui->btc_hint->setToolTip("Alice is expected to send monero lock after one btc confirmation,\nswap is cancelable after 72 btc confirmations,\nyou will lose your funds if you don't refund before 144 confirmations\n\nResumed swaps may not have accurate numbers for confirmations!!!");
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
            QString message = line["fields"]["message"].toString();
            if (message.contains("Connected to Alice")){
                qDebug() << "Successfully connected";
                this->logLine(line["fields"].toString());
            } else if (!line["fields"]["deposit_address"].toString().isEmpty()){
                qDebug() << "Deposit to btc to segwit address";
                QString address = line["fields"]["deposit_address"].toString();
                fundDialog = new AtomicFundDialog(this,  "Deposit BTC to this address", address);
                fundDialog->updateMin(min);
                fundDialog->update();
                //dialog->setModal(true);
                fundDialog->show();
            } else if (message.startsWith("Received Bitcoin")){
                this->updateStatus(line["fields"]["new_balance"].toString().split(" ")[0] + " BTC received, starting swap");
                QString entry = line["span"]["swap_id"].toString() + ":" + QDateTime::currentDateTime().toString("dd.MM.yyyy.hh.mm.ss");
                qDebug() << "Swap logged as ";
                qDebug() << entry;
                QVariantList past = conf()->get(Config::pendingSwap).toList();
                past.append(entry);
                conf()->set(Config::pendingSwap,past);
                fundDialog->close();
                qDebug() << "Spawn atomic swap progress dialog";
                this->show();
            } else if ( QString confs = line["fields"]["seen_confirmations"].toString(); !confs.isEmpty()){
                qDebug() << "Updating xmrconfs " + confs;
                this->updateXMRConf(confs.toInt());
            } else if (QString::compare(message, "Bitcoin transaction status changed")==0){
                QString status = line["fields"]["new_status"].toString();
                auto parts = status.split(" ");
                if (parts.length() == 2){
                    this->updateStatus("Found txid " + line["fields"]["txid"].toString() + " in mempool");

                }else {
                    auto confirmations = parts[2].toInt();
                    this->updateBTCConf(confirmations);
                }
            } else if (message.startsWith("Swap completed")){
                QVariantList past = conf()->get(Config::pendingSwap).toList();
                past.removeLast();
                conf()->set(Config::pendingSwap, past);
                this->updateStatus("Swap has successfully completed you can close this window now");
            } else if (QString::compare(message,"Advancing state")==0){
                this->updateStatus("State of swap has advanced to " + line["fields"]["state"].toString());
            } else if (QString refund = line["fields"]["kind"].toString(); QString::compare(refund,"refund")==0){
                QString txid = line["fields"]["txid"].toString();
                QString id = line["span"]["swap_id"].toString();
                QStringList past = conf()->get(Config::pendingSwap).toStringList();
                for(int i=0;i<past.length();i++){
                    if(QString::compare(past[i].split(":")[0],id)==0) {
                        past.remove(i);
                        break;
                    }
                }
                conf()->set(Config::pendingSwap, past);
                QMessageBox::information(this,"Cancel and Refund","Swap refunded succesfully with txid " + txid);
            } else if ( QString::compare(message, "API call resulted in an error")==0){
                QString err = line["fields"]["err"].toString().split("\n")[0].split(":")[1];
                QMessageBox::warning(this, "Cancel and Refund", "Time lock hasn't expired yet so cancel failed. Try again in " + err + "blocks");
            } else if (QString latest_version = line["fields"]["latest_version"].toString(); !latest_version.isEmpty()){
                QMessageBox::warning(this, "Outdated swap version","A newer version of COMIT xmr-btc swap tool is available, delete current binary and re auto install to upgrade");
                conf()->set(Config::swapVersion,latest_version);
            } else if (message.startsWith("Acquiring swap lock") && QString::compare("Resume",line["span"]["method_name"].toString())==0){
                updateStatus("Beginning resumption of previous swap");
                this->show();
            } else if (message.startsWith("Deposit at least")){
                min = message.split(" ")[3];
            }
        }
    });

    swap->start(conf()->get(Config::swapPath).toString(),arguments);
    qDebug() << "process started";
}
AtomicSwap::~AtomicSwap() {
    for (const auto& proc : *procList){
        proc->kill();
    }
    if(conf()->get(Config::operatingSystem)=="WINDOWS"){
        (new QProcess)->start("tskill", QStringList{"monero-wallet-rpc"});
    }else {
        if (constants::networkType==NetworkType::STAGENET){
            (new QProcess)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +"/testnet/monero/monero-wallet-rpc"});
        } else {
            (new QProcess)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                             "/mainnet/monero/monero-wallet-rpc"});
        }
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
