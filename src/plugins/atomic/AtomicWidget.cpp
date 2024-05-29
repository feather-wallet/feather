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
#include "utils/config.h"
#include "utils/WebsocketNotifier.h"

AtomicWidget::AtomicWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::AtomicWidget)
        , o_model(new OfferModel(this))
        , offerList(new QList<QSharedPointer<OfferEntry>>())
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

        auto m_instance = Config::instance();
        QStringList pointList = m_instance->get(Config::rendezVous).toStringList();
        for(QString point :pointList)
            AtomicWidget::list(point);

        /*
        QList<QFuture<void>> tempList;
        for (int i=0; i<pointList.size();i++){
            tempList.append(QtConcurrent::run([this, pointList, i]{
                return AtomicWidget::list(pointList[i]);
            }));
        }

        sleep(130);
        for (int i=0; i<pointList.size();i++){
            qDebug() << "Starting to read offers";
            auto offers = tempList[i].result();
            for (auto offer: offers){
                offerList->append(offer);
            }
        }
        qDebug() << "done";

        /*for(auto offer: AtomicWidget::list(Config::instance()->get(Config::rendezVous).toStringList()[0])){
            offerList->append(offer);
        }
                 o_model->updateOffers(*offerList);

         */
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



void AtomicWidget::updateStatus() {

}

void AtomicWidget::list(QString rendezvous) {
    QStringList arguments;
    QList<QSharedPointer<OfferEntry>> list;
    auto m_instance = Config::instance();
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
    swap->setReadChannel(QProcess::StandardOutput);
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

AtomicWidget::~AtomicWidget() = default;