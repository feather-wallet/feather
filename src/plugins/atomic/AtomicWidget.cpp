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
#include "dialog/QrCodeDialog.h"

AtomicWidget::AtomicWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::AtomicWidget)
        , m_instance(Config::instance())
        , o_model(new OfferModel(this))
        , offerList(new QList<QSharedPointer<OfferEntry>>())
        , swapDialog(new AtomicSwap(this))
        , procList(new QList<QSharedPointer<QProcess>>())
{
    ui->setupUi(this);


    // validator/locale for input
    QString amount_rx = R"(^\d{0,8}[\.]\d{0,12}$)";
    QRegularExpression rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->offerBookTable->setModel(o_model);
    ui->offerBookTable->setSortingEnabled(true);
    ui->offerBookTable->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    ui->offerBookTable->verticalHeader()->setVisible(false);
    ui->offerBookTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->btn_configure->setEnabled(true);

    if (!Config::instance()->get(Config::swapPath).toString().isEmpty())
        ui->meta_label->setText("Refresh offer book before swapping to prevent errors");

    connect(ui->btn_configure, &QPushButton::clicked, this, &AtomicWidget::showAtomicConfigureDialog);

    connect(ui->btn_refreshOffer, &QPushButton::clicked, this, [this]{
        offerList->clear();

        ui->meta_label->setText("Updating offer book this may take a bit, if no offers appear after a while try refreshing again");

        QStringList pointList = m_instance->get(Config::rendezVous).toStringList();
        for(QString point :pointList)
            AtomicWidget::list(point);
    });

    connect(ui->btn_swap, &QPushButton::clicked, this, [this]{
        auto rows = ui->offerBookTable->selectionModel()->selectedRows();
        if (rows.size() < 1){
            ui->meta_label->setText("You must select an offer to use for swap, refresh if there aren't any");
        } else {
            QModelIndex index = rows.at(0);
            QString seller = index.sibling(index.row(), 3).data().toString();
            //Add proper error checking on ui input after rest of swap is implemented
            QString btcChange = ui->change_address->text();
            QString xmrReceive = ui->xmr_address->text();
            showAtomicSwapDialog();
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
            QStringList copy = Config::instance()->get(Config::rendezVous).toStringList();
            copy.append(text);
            Config::instance()->set(Config::rendezVous,copy);
        }
    });


    this->updateStatus();
}



void AtomicWidget::skinChanged() {
}

void AtomicWidget::showAtomicConfigureDialog() {
    AtomicConfigDialog dialog{this};

    if (dialog.exec() == QDialog::Accepted) {

    }
}

void AtomicWidget::showAtomicSwapDialog() {
    swapDialog->show();
}


void AtomicWidget::updateStatus() {

}

void AtomicWidget::runSwap(QString seller, QString btcChange, QString xmrReceive) {
    qDebug() << "starting swap";
    QStringList  arguments;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
    // Remove after testing
    //arguments << "--testnet";
    arguments << "-j";
    arguments << "buy-xmr";
    arguments << "--change-address";
    arguments << btcChange;
    arguments << "--receive-address";
    arguments << xmrReceive;
    arguments << "--seller";
    arguments << seller;
    arguments << "--tor-socks5-port";
    arguments << m_instance->get(Config::socks5Port).toString();

    auto *swap = new QProcess();
    procList->append(QSharedPointer<QProcess>(swap));

    swap->setReadChannel(QProcess::StandardError);
    connect(swap, &QProcess::readyReadStandardError,this, [this, swap] {
        while(swap->canReadLine()){
            QJsonParseError err;
            const QByteArray& rawline = swap->readLine();
            QJsonDocument line = QJsonDocument::fromJson(rawline, &err);
            qDebug() << rawline;
            if (line["fields"]["message"].toString().contains("Connected to Alice")){
                qDebug() << "Succesfully connected";
                swapDialog->logLine(line["fields"].toString());
            } else if (!line["fields"]["deposit_address"].toString().isEmpty()){
                qDebug() << "Deposit to btc to segwit address";
                QString address = line["fields"]["deposit_address"].toString();
                QrCode qrc(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH);
                swapDialog->updateStatus("Add money to this address\n" + address);
                QrCodeDialog dialog(this, &qrc,  "Deposit BTC to this address");
                dialog.show();
            } else{

            }
            //Insert line conditionals here
        }
    });
    swap->start(m_instance->get(Config::swapPath).toString(),arguments);
    qDebug() << "process started";

}

void AtomicWidget::list(QString rendezvous) {
    QStringList arguments;
    QList<QSharedPointer<OfferEntry>> list;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
    // Remove after testing
    //arguments << "--testnet";
    arguments << "-j";
    arguments << "list-sellers";
    arguments << "--tor-socks5-port";
    arguments << m_instance->get(Config::socks5Port).toString();
    arguments << "--rendezvous-point";
    arguments << rendezvous;
    auto *swap = new QProcess();
    procList->append(QSharedPointer<QProcess>(swap));
    swap->setReadChannel(QProcess::StandardError);
    //swap->start(m_instance->get(Config::swapPath).toString(), arguments);
    connect(swap, &QProcess::finished, this, [this, swap]{
        QJsonDocument parsedLine;
        QJsonParseError parseError;
        QList<QSharedPointer<OfferEntry>> list;
        qDebug() << "Subprocess has finished";
        auto output = QString::fromLocal8Bit(swap->readAllStandardError());
        qDebug() << "Crashes before splitting";
        auto lines = output.split(QRegularExpression("[\r\n]"),Qt::SkipEmptyParts);
        qDebug() << lines.size();
        qDebug() << "parsing Output";


        for(auto line : lines){
            qDebug() << line;
            if(line.contains("status")){
                qDebug() << "status contained";
                parsedLine = QJsonDocument::fromJson(line.toLocal8Bit(), &parseError );
                if (parsedLine["fields"]["status"].toString().contains("Online")){
                    bool skip = false;
                    auto  entry = new OfferEntry(parsedLine["fields"]["price"].toString().split( ' ')[0].toDouble(),parsedLine["fields"]["min_quantity"].toString().split(' ')[0].toDouble(),parsedLine["fields"]["max_quantity"].toString().split(' ')[0].toDouble(), parsedLine["fields"]["address"].toString());
                    for(auto post : *offerList){
                        if(std::equal(entry->address.begin(), entry->address.end(),post->address.begin(),post->address.end()))
                            skip = true;
                    }
                    if (!skip) {
                        ui->meta_label->setText("Updated offer book");
                        offerList->append(QSharedPointer<OfferEntry>(entry));
                    }
                    qDebug() << entry;
                }
            }
            qDebug() << "next line";
        }
        qDebug() << "exits fine";
        swap->close();
        o_model->updateOffers(*offerList);
        return list;
    });
    swap->start(m_instance->get(Config::swapPath).toString(), arguments);
    //swap->waitForFinished(120000);



}

AtomicWidget::~AtomicWidget() {
    qDebug()<< "EXiting widget!!";
    delete o_model;
    delete offerList;
    for (auto proc : *procList){
        if(!proc->atEnd())
            proc->terminate();
    }
    if(QString::compare("WINDOWS",m_instance->get(Config::operatingSystem).toString()) != 0) {
        qDebug() << "Closing monero-wallet-rpc";
        (new QProcess)->start("pkill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                         "/mainnet/monero/monero-wallet-rpc"});
    }
        delete m_instance;
    delete procList;
};