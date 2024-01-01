// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "CalcWidget.h"
#include "ui_CalcWidget.h"

#include <QList>

#include "CalcConfigDialog.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/config.h"
#include "utils/WebsocketNotifier.h"

CalcWidget::CalcWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalcWidget)
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

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &CalcWidget::onPricesReceived);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &CalcWidget::onPricesReceived);

    connect(ui->lineFrom, &QLineEdit::textEdited, this, [this]{this->convert(false);});
    connect(ui->lineTo,   &QLineEdit::textEdited, this, [this]{this->convert(true);});

    connect(ui->comboCalcFrom, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(false);});
    connect(ui->comboCalcTo,   QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(false);});

    connect(ui->btn_configure, &QPushButton::clicked, this, &CalcWidget::showCalcConfigureDialog);

    QTimer::singleShot(1, [this]{
        this->skinChanged();
    });

    m_statusTimer.start(5000);
    connect(&m_statusTimer, &QTimer::timeout, this, &CalcWidget::updateStatus);
    QPixmap warningIcon = QPixmap(":/assets/images/warning.png");
    ui->icon_warning->setPixmap(warningIcon.scaledToWidth(32, Qt::SmoothTransformation));

    this->updateStatus();
}

void CalcWidget::convert(bool reverse) {
    if (!m_comboBoxInit)
        return;

    auto lineFrom = reverse ? ui->lineTo : ui->lineFrom;
    auto lineTo = reverse ? ui->lineFrom : ui->lineTo;

    auto comboFrom = reverse ? ui->comboCalcTo : ui->comboCalcFrom;
    auto comboTo = reverse ? ui->comboCalcFrom : ui->comboCalcTo;

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

void CalcWidget::onPricesReceived() {
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

void CalcWidget::initComboBox() {
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

    this->setupComboBox(ui->comboCalcFrom, filteredCryptoKeys, filteredFiatKeys);
    this->setupComboBox(ui->comboCalcTo,   filteredCryptoKeys, filteredFiatKeys);

    ui->comboCalcFrom->setCurrentIndex(ui->comboCalcFrom->findText("XMR"));

    if (!preferredFiat.isEmpty()) {
        ui->comboCalcTo->setCurrentIndex(ui->comboCalcTo->findText(preferredFiat));
    } else {
        ui->comboCalcTo->setCurrentIndex(ui->comboCalcTo->findText("USD"));
    }
}

void CalcWidget::skinChanged() {
    ui->imageExchange->setMode(ColorScheme::hasDarkBackground(this));
}

void CalcWidget::showCalcConfigureDialog() {
    CalcConfigDialog dialog{this};

    if (dialog.exec() == QDialog::Accepted) {
        this->initComboBox();
    }
}

void CalcWidget::setupComboBox(QComboBox *comboBox, const QStringList &crypto, const QStringList &fiat) {
    comboBox->clear();
    comboBox->addItems(crypto);
    comboBox->insertSeparator(comboBox->count());
    comboBox->addItems(fiat);
}

void CalcWidget::updateStatus() {
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

CalcWidget::~CalcWidget() = default;