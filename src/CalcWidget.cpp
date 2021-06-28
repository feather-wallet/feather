// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CalcWidget.h"
#include "ui_CalcWidget.h"

#include <QList>

#include "dialog/CalcConfigDialog.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/config.h"

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
    QLocale lo(QLocale::C);
    lo.setNumberOptions(QLocale::RejectGroupSeparator);
    auto dv = new QDoubleValidator(0.0, 2147483647, 10, this); // [0, 32bit max], 10 decimals of precision
    dv->setNotation(QDoubleValidator::StandardNotation);
    dv->setLocale(lo);
    ui->lineFrom->setValidator(dv);
    ui->lineTo->setValidator(dv);

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
}

void CalcWidget::initComboBox() {
    QList<QString> cryptoKeys = appData()->prices.markets.keys();
    QList<QString> fiatKeys = appData()->prices.rates.keys();

    QStringList enabledCrypto = config()->get(Config::cryptoSymbols).toStringList();
    QStringList filteredCryptoKeys;
    for (const auto& symbol : cryptoKeys) {
        if (enabledCrypto.contains(symbol)) {
            filteredCryptoKeys.append(symbol);
        }
    }

    QStringList enabledFiat = config()->get(Config::fiatSymbols).toStringList();
    auto preferredFiat = config()->get(Config::preferredFiatCurrency).toString();
    if (!enabledFiat.contains(preferredFiat) && fiatKeys.contains(preferredFiat)) {
        enabledFiat.append(preferredFiat);
        config()->set(Config::fiatSymbols, enabledFiat);
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

CalcWidget::~CalcWidget() = default;