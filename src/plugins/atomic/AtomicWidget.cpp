// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicWidget.h"
#include "ui_AtomicWidget.h"

#include <QList>
#include <QProcess>

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

    ui->imageExchange->setBackgroundRole(QPalette::Base);
    ui->imageExchange->setAssets(":/assets/images/exchange.png", ":/assets/images/exchange_white.png");
    ui->imageExchange->setScaledContents(true);
    ui->imageExchange->setFixedSize(26, 26);

    // validator/locale for input
    QString amount_rx = R"(^\d{0,8}[\.]\d{0,12}$)";
    QRegularExpression rx;
    rx.setPattern(amount_rx);
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->lineFrom->setValidator(validator);
    ui->lineTo->setValidator(validator);
    ui->offerBookTable->setModel(o_model);
    ui->offerBookTable->setSortingEnabled(true);
    ui->offerBookTable->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    ui->offerBookTable->verticalHeader()->setVisible(false);
    ui->offerBookTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->offerBookTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    //ui->offerBookTable->
    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &AtomicWidget::onPricesReceived);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &AtomicWidget::onPricesReceived);

    connect(ui->lineFrom, &QLineEdit::textEdited, this, [this]{this->convert(false);});
    connect(ui->lineTo,   &QLineEdit::textEdited, this, [this]{this->convert(true);});

    connect(ui->comboAtomicFrom, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(false);});
    connect(ui->comboAtomicTo,   QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(false);});

    connect(ui->btn_configure, &QPushButton::clicked, this, &AtomicWidget::showAtomicConfigureDialog);

    connect(ui->btn_refreshOffer, &QPushButton::clicked, this, [this]{
        offerList->clear();

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

    QTimer::singleShot(1, [this]{
        this->skinChanged();
    });

    m_statusTimer.start(5000);
    connect(&m_statusTimer, &QTimer::timeout, this, &AtomicWidget::updateStatus);
    QPixmap warningIcon = QPixmap(":/assets/images/warning.png");
    ui->icon_warning->setPixmap(warningIcon.scaledToWidth(32, Qt::SmoothTransformation));

    this->updateStatus();
}

void AtomicWidget::convert(bool reverse) {
    if (!m_comboBoxInit)
        return;

    auto lineFrom = reverse ? ui->lineTo : ui->lineFrom;
    auto lineTo = reverse ? ui->lineFrom : ui->lineTo;

    auto comboFrom = reverse ? ui->comboAtomicTo : ui->comboAtomicFrom;
    auto comboTo = reverse ? ui->comboAtomicFrom : ui->comboAtomicTo;

    QString symbolFrom = comboFrom->itemText(comboFrom->currentIndex());
    QString symbolTo = comboTo->itemText(comboTo->currentIndex());

    if (symbolFrom == symbolTo) {
        lineTo->setText(lineFrom->text());
    }

    QString amountStr = lineFrom->text();
    double amount = amountStr.toDouble();
    double result = appData()->prices.convert(symbolFrom, symbolTo, amount);

    int precision = 10;
    if (appData()->prices.rates.contains(symbolTo))
        precision = 2;

    lineTo->setText(QString::number(result, 'f', precision));
}

void AtomicWidget::onPricesReceived() {
    if (m_comboBoxInit)
        return;

    QList<QString> cryptoKeys = appData()->prices.markets.keys();
    QList<QString> fiatKeys = appData()->prices.rates.keys();
    if (cryptoKeys.empty() || fiatKeys.empty())
        return;

    ui->btn_configure->setEnabled(true);
    this->initComboBox();
    m_comboBoxInit = true;
    this->updateStatus();
}

void AtomicWidget::initComboBox() {
    QList<QString> cryptoKeys = appData()->prices.markets.keys();
    QList<QString> fiatKeys = appData()->prices.rates.keys();

    QStringList enabledCrypto = conf()->get(Config::cryptoSymbols).toStringList();
    QStringList filteredCryptoKeys;
    for (const auto& symbol : cryptoKeys) {
        if (enabledCrypto.contains(symbol)) {
            filteredCryptoKeys.append(symbol);
        }
    }

    QStringList enabledFiat = conf()->get(Config::fiatSymbols).toStringList();
    auto preferredFiat = conf()->get(Config::preferredFiatCurrency).toString();
    if (!enabledFiat.contains(preferredFiat) && fiatKeys.contains(preferredFiat)) {
        enabledFiat.append(preferredFiat);
        conf()->set(Config::fiatSymbols, enabledFiat);
    }
    QStringList filteredFiatKeys;
    for (const auto &symbol : fiatKeys) {
        if (enabledFiat.contains(symbol)) {
            filteredFiatKeys.append(symbol);
        }
    }

    this->setupComboBox(ui->comboAtomicFrom, filteredCryptoKeys, filteredFiatKeys);
    this->setupComboBox(ui->comboAtomicTo,   filteredCryptoKeys, filteredFiatKeys);

    ui->comboAtomicFrom->setCurrentIndex(ui->comboAtomicFrom->findText("XMR"));

    if (!preferredFiat.isEmpty()) {
        ui->comboAtomicTo->setCurrentIndex(ui->comboAtomicTo->findText(preferredFiat));
    } else {
        ui->comboAtomicTo->setCurrentIndex(ui->comboAtomicTo->findText("USD"));
    }
}

void AtomicWidget::skinChanged() {
    ui->imageExchange->setMode(ColorScheme::hasDarkBackground(this));
}

void AtomicWidget::showAtomicConfigureDialog() {
    AtomicConfigDialog dialog{this};

    if (dialog.exec() == QDialog::Accepted) {
        this->initComboBox();
    }
}

void AtomicWidget::setupComboBox(QComboBox *comboBox, const QStringList &crypto, const QStringList &fiat) {
    comboBox->clear();
    comboBox->addItems(crypto);
    comboBox->insertSeparator(comboBox->count());
    comboBox->addItems(fiat);
}

void AtomicWidget::updateStatus() {
    if (!m_comboBoxInit) {
        ui->label_warning->setText("Waiting on exchange data.");
        ui->frame_warning->show();
    }
    else if (websocketNotifier()->stale(10)) {
        ui->label_warning->setText("No new exchange rates received for over 10 minutes.");
        ui->frame_warning->show();
    }
    else {
        ui->frame_warning->hide();
    }
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
                    OfferEntry  entry = {parsedLine["fields"]["price"].toDouble(),parsedLine["fields"]["min_quantity"].toDouble(),parsedLine["fields"]["max_quantity"].toDouble(), parsedLine["fields"]["address"].toString()};
                    offerList->append(QSharedPointer<OfferEntry>(&entry));
                    qDebug() << &entry;
                }
            }
            qDebug() << "next line";
        }
        qDebug() << "exits fine";
        swap->close();
        return list;
    });
    swap->start("/home/dev/.config/feather/swap", arguments);
    //swap->waitForFinished(120000);



}

AtomicWidget::~AtomicWidget() = default;