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
        , m_instance(Config::instance())
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

    if (!m_instance->get(Config::swapPath).toString().isEmpty())
        ui->meta_label->setText("Refresh offer book before swapping to prevent errors");

    connect(ui->btn_configure, &QPushButton::clicked, this, &AtomicWidget::showAtomicConfigureDialog);

    connect(ui->btn_refreshOffer, &QPushButton::clicked, this, [this]{
        offerList->clear();

        ui->meta_label->setText("Updating offer book this may take a bit, if no offers appear after a while try refreshing again");

        QStringList pointList = m_instance->get(Config::rendezVous).toStringList();
        for(const QString& point :pointList)
            AtomicWidget::list(point);
    });

    connect(ui->btn_swap, &QPushButton::clicked, this, [this]{
        auto rows = ui->offerBookTable->selectionModel()->selectedRows();
        clean();
        // UNCOMENT after testing
        //if (rows.size() < 1){
        //    ui->meta_label->setText("You must select an offer to use for swap, refresh if there aren't any");
        //} else {
            //QModelIndex index = rows.at(0);
            //QString seller = index.sibling(index.row(), 3).data().toString();
            QString seller = "test";
            //Add proper error checking on ui input after rest of swap is implemented
            QString btcChange = ui->change_address->text();
            QString xmrReceive = ui->xmr_address->text();
            sleep(1);
            runSwap(seller,btcChange, xmrReceive);
        //}
    });

    connect(ui->btn_addRendezvous, &QPushButton::clicked, this, [this]{
        //offerList = new QList<QSharedPointer<OfferEntry>>;
        bool ok;
        QString text = QInputDialog::getText(this, tr("Add new rendezvous point"),
                                             tr("p2p multi address of rendezvous point"), QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            QStringList copy = m_instance->get(Config::rendezVous).toStringList();
            copy.append(text);
            m_instance->set(Config::rendezVous,copy);
        }
    });


    //Remove after testing
    //QVariant var;
    //var.setValue(HistoryEntry {QDateTime::currentDateTime(),"test-id"});
    //m_instance->set(Config::pendingSwap, QVariantList{var});
    //auto recd = new AtomicRecoverDialog();
    //if (!recd->historyEmpty()){
    //    recd->show();
    //}
    this->updateStatus();
}



void AtomicWidget::skinChanged() {
}

void AtomicWidget::showAtomicConfigureDialog() {
    AtomicConfigDialog dialog{this};
    dialog.exec();
}

void AtomicWidget::showAtomicSwapDialog() {
    swapDialog->show();
}

void AtomicWidget::updateStatus() {

}

void AtomicWidget::runSwap(const QString& seller, const QString& btcChange, const QString& xmrReceive) {
    qDebug() << "starting swap";
    QStringList  arguments;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
    // Remove after testing
    arguments << "--testnet";
    arguments << "--debug";
    arguments << "-j";
    arguments << "buy-xmr";
    arguments << "--change-address";
    arguments << "tb1qzndh6u8qgl2ee4k4gl9erg947g67hyx03vvgen";
    //arguments << btcChange;
    arguments << "--receive-address";
    arguments << "78YnzFTp3UUMgtKuAJCP2STcbxRZPDPveJ5YGgfg5doiPahS9suWF1r3JhKqjM1McYBJvu8nhkXExGfXVkU6n5S6AXrg4KP";
    //arguments << xmrReceive;
    arguments << "--seller";
    arguments << "/ip4/127.0.0.1/tcp/9939/p2p/12D3KooW9yDFYojXnZRdqS9UXcfP2amgwoYdSjujwWdRw4LTSdWw";
    // Remove after testing
    arguments << "--electrum-rpc";
    arguments << "tcp://127.0.0.1:50001";
    arguments << "--bitcoin-target-block";
    arguments << "1";
    arguments << "--monero-daemon-address";
    arguments << "node.monerodevs.org:38089";
    // Uncomment after testing
    //arguments << seller;
    arguments << "--tor-socks5-port";
    arguments << m_instance->get(Config::socks5Port).toString();
    swapDialog->runSwap(arguments);

}

void AtomicWidget::list(const QString& rendezvous) {
    QStringList arguments;
    arguments << "--data-base-dir";
    arguments << Config::defaultConfigDir().absolutePath();
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


        for(const auto& line : lines){
            qDebug() << line;
            if(line.contains("status")){
                qDebug() << "status contained";
                parsedLine = QJsonDocument::fromJson(line.toLocal8Bit(), &parseError );
                if (parsedLine["fields"]["status"].toString().contains("Online")){
                    bool skip = false;
                    auto  entry = new OfferEntry(parsedLine["fields"]["price"].toString().split( ' ')[0].toDouble(),parsedLine["fields"]["min_quantity"].toString().split(' ')[0].toDouble(),parsedLine["fields"]["max_quantity"].toString().split(' ')[0].toDouble(), parsedLine["fields"]["address"].toString());
                    for(const auto& post : *offerList){
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
    qDebug()<< "Exiting widget!!";
    delete swapDialog;
    delete o_model;
    delete offerList;
    clean();
    delete m_instance;
    delete procList;
}

void AtomicWidget::clean() {
    for (const auto& proc : *procList){
            proc->kill();
    }
    if(QString::compare("WINDOWS",m_instance->get(Config::operatingSystem).toString()) != 0) {
        qDebug() << "Closing monero-wallet-rpc";
        (new QProcess)->start("kill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                         "/mainnet/monero/monero-wallet-rpc"});
        (new QProcess)->start("kill", QStringList{"-f", Config::defaultConfigDir().absolutePath() +
                                                         "/testnet/monero/monero-wallet-rpc"});
    }
}



