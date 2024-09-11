// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicWidget.h"
#include "ui_AtomicWidget.h"

#include <QList>
#include <QProcess>
#include <QInputDialog>

#include "AtomicConfigDialog.h"
#include "OfferModel.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/WebsocketNotifier.h"
#include "AtomicFundDialog.h"

AtomicWidget::AtomicWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::AtomicWidget)
        , o_model(new OfferModel(this))
        , offerList(new QList<QSharedPointer<OfferEntry>>())
        , swapDialog(new AtomicSwap(this))
        , procList(new QList<QSharedPointer<QProcess>>())
        , fundDialog(new AtomicFundDialog(this))
        , recoverDialog(new AtomicRecoverDialog(this))
{
    ui->setupUi(this);


    // validator/locale for input
    QString amount_rx = R"(^\d{0,8}[\.]\d{0,12}$)";
    QRegularExpression rx;
    rx.setPattern(amount_rx);
    ui->offerBookTable->setModel(o_model);
    ui->offerBookTable->setSortingEnabled(true);
    ui->offerBookTable->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    ui->offerBookTable->verticalHeader()->setVisible(false);
    ui->offerBookTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->btn_configure->setEnabled(true);

    if (!conf()->get(Config::swapPath).toString().isEmpty())
        ui->meta_label->setText("Refresh offer book before swapping to prevent errors");

    connect(ui->btn_configure, &QPushButton::clicked, this, &AtomicWidget::showAtomicConfigureDialog);

    connect(ui->btn_refreshOffer, &QPushButton::clicked, this, [this]{
        offerList->clear();

        ui->meta_label->setText("Updating offer book this may take a bit, if no offers appear after a while try refreshing again");

        QStringList pointList = conf()->get(Config::rendezVous).toStringList();
        for(const QString& point :pointList)
            AtomicWidget::list(point);
    });

    connect(ui->btn_swap, &QPushButton::clicked, this, [this]{
        auto rows = ui->offerBookTable->selectionModel()->selectedRows();
        clean();
        if (rows.size() < 1){
            ui->meta_label->setText("You must select an offer to use for swap, refresh if there aren't any");
        } else {
            QModelIndex index = rows.at(0);
            QString seller = index.sibling(index.row(), 3).data().toString();
            //Add proper error checking on ui input after rest of swap is implemented
            QString btcChange = ui->change_address->text();
            QRegularExpression btcMain("^(bc1)[a-zA-HJ-NP-Z0-9]{39}$");
            QRegularExpression btcTest("^(tb1)[a-zA-HJ-NP-Z0-9]{39}$");
            QString xmrReceive = ui->xmr_address->text();
            if(xmrReceive.isEmpty()) {
                QMessageBox::warning(this, "Warning", "XMR receive address is required to start swap");
                return;
            }
            QRegularExpression xmrMain("^[48][0-9AB][1-9A-HJ-NP-Za-km-z]{93}");
            QRegularExpression xmrStage("^[57][0-9AB][1-9A-HJ-NP-Za-km-z]{93}");
            if (constants::networkType==NetworkType::STAGENET){
                if(!btcChange.isEmpty() && !btcTest.match(btcChange).hasMatch()){
                    QMessageBox::warning(this, "Warning","BTC change address is wrong, not a bech32 segwit address, or on wrong network");
                    return;
                }
                if(!xmrStage.match(xmrReceive).hasMatch()){
                    QMessageBox::warning(this, "Warning","XMR receive address is improperly formated or on wrong network");
                    return;
                }
            } else {
                if(!btcChange.isEmpty() && !btcMain.match(btcChange).hasMatch()){
                    QMessageBox::warning(this, "Warning","BTC change address is wrong, not a bech32 segwit address,or on wrong network");
                    return;
                }
                if(!xmrMain.match(xmrReceive).hasMatch()){
                    QMessageBox::warning(this, "Warning","XMR receive address is improperly formated or on wrong network");
                    return;
                }
            }

            runSwap(seller,btcChange, xmrReceive);
        }
    });

    connect(ui->btn_addRendezvous, &QPushButton::clicked, this, [this]{
        //offerList = new QList<QSharedPointer<OfferEntry>>;
        bool ok;
        QString text = QInputDialog::getText(this, tr("Add new rendezvous point"),
                                             tr("p2p multi address of rendezvous point"), QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            QStringList copy = conf()->get(Config::rendezVous).toStringList();
            copy.append(text);
            conf()->set(Config::rendezVous,copy);
        }
    });

    auto recd = new AtomicRecoverDialog(this);
    if (!recd->historyEmpty()){
        recd->show();
    }
}



void AtomicWidget::skinChanged() {
}

void AtomicWidget::showAtomicConfigureDialog() {
    AtomicConfigDialog dialog{this};
    dialog.exec();
}


void AtomicWidget::runSwap(const QString& seller, const QString& btcChange, const QString& xmrReceive) {
    qDebug() << "starting swap";
    clean();
    QStringList  arguments;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
    if (constants::networkType==NetworkType::STAGENET){
        arguments << "--testnet";
    }
    arguments << "--debug";
    arguments << "-j";
    arguments << "buy-xmr";
    arguments << "--change-address";
    arguments << btcChange;
    arguments << "--receive-address";
    arguments << xmrReceive;

    //Doesn't work cause wallet rpc
    /*
    auto nodes = conf()->get(Config::nodes).toJsonObject();
    if (nodes.isEmpty()) {
        auto jsonData = conf()->get(Config::nodes).toByteArray();
        if (Utils::validateJSON(jsonData)) {
            auto doc = QJsonDocument::fromJson(jsonData);
            nodes = doc.object();
        }
    }
    arguments << "--monero-daemon-address";
    arguments << nodes.value("0").toObject()["ws"].toArray()[0].toString();
     */
    arguments << "--seller";
    arguments << seller;
    if(conf()->get(Config::proxy).toInt() != Config::Proxy::None) {
        arguments << "--tor-socks5-port";
        arguments << conf()->get(Config::socks5Port).toString();
    }
    swapDialog->runSwap(arguments);

}

void AtomicWidget::list(const QString& rendezvous) {
    QStringList arguments;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
    arguments << "--debug";
    if (constants::networkType==NetworkType::STAGENET){
        arguments << "--testnet";
    }
    arguments << "-j";
    arguments << "list-sellers";
    if(conf()->get(Config::proxy).toInt() != Config::Proxy::None) {
        arguments << "--tor-socks5-port";
        arguments << conf()->get(Config::socks5Port).toString();
    }
    arguments << "--rendezvous-point";
    arguments << rendezvous;
    qDebug() << "rendezvous point: " + rendezvous;
    auto *swap = new QProcess();
    procList->append(QSharedPointer<QProcess>(swap));
    swap->setReadChannel(QProcess::StandardError);
    connect(swap, &QProcess::finished, this, [this, swap]{
        QJsonDocument parsedLine;
        QJsonParseError parseError;
        QList<QSharedPointer<OfferEntry>> list;
        auto output = QString::fromLocal8Bit(swap->readAllStandardError());
        auto lines = output.split(QRegularExpression("[\r\n]"),Qt::SkipEmptyParts);


        for(const auto& line : lines){
            qDebug() << line;
            if(line.contains("status")){
                parsedLine = QJsonDocument::fromJson(line.toLocal8Bit(), &parseError );
                if (parsedLine["fields"]["status"].toString().contains("Online")){
                    bool skip = false;
                    auto  entry = new OfferEntry(parsedLine["fields"]["price"].toString().split( ' ')[0].toDouble(),parsedLine["fields"]["min_quantity"].toString().split(' ')[0].toDouble(),parsedLine["fields"]["max_quantity"].toString().split(' ')[0].toDouble(), parsedLine["fields"]["address"].toString());
                    for(const auto& post : *offerList){
                        if(entry->max == 0 || std::equal(entry->address.begin(), entry->address.end(),post->address.begin(),post->address.end()))
                            skip = true;
                    }
                    if (!skip) {
                        ui->meta_label->setText("Updated offer book");
                        offerList->append(QSharedPointer<OfferEntry>(entry));
                    }
                }
            } else if (line.contains("GLIBC_")){
                QMessageBox::critical(this, "GLIBC outdated", "Upgrade your GLIBC to at least 2.32 to use this tool");
                clean();
            }
        }
        swap->close();
        o_model->updateOffers(*offerList);
        return list;
    });
    swap->start(conf()->get(Config::swapPath).toString(), arguments);



}

AtomicWidget::~AtomicWidget() {
    qDebug()<< "Exiting widget!!";
    delete swapDialog;
    delete o_model;
    delete offerList;
    clean();
    delete procList;
}

void AtomicWidget::clean() {
    for (const auto& proc : *procList){
        proc->kill();
    }
    auto cleanWallet =  new QProcess;
    auto cleanSwap = new QProcess;
    if(conf()->get(Config::operatingSystem)=="WINDOWS"){
        (cleanWallet)->start("tskill", QStringList{"monero-wallet-rpc"});
        (cleanWallet)->start("tskill", QStringList{"swap"});
    }else {
        if (constants::networkType==NetworkType::STAGENET){
            (cleanWallet)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +"/testnet/monero/monero-wallet-rpc"});
            (cleanSwap)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                          "/*"});
        } else {
            (cleanWallet)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                             "/mainnet/monero/monero-wallet-rpc"});
            (cleanSwap)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                             "/*"});
        }
    }
    cleanWallet->waitForFinished();
    cleanSwap->waitForFinished();
}



