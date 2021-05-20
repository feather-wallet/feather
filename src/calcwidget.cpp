// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QList>

#include "calcwidget.h"
#include "ui_calcwidget.h"
#include "utils/ColorScheme.h"
#include "utils/AppData.h"
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

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &CalcWidget::initComboBox);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &CalcWidget::initComboBox);

    connect(ui->lineFrom, &QLineEdit::textEdited, this, [this]{this->convert(false);});
    connect(ui->lineTo,   &QLineEdit::textEdited, this, [this]{this->convert(true);});

    connect(ui->comboCalcFrom, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(false);});
    connect(ui->comboCalcTo,   QOverload<int>::of(&QComboBox::currentIndexChanged), [this]{this->convert(true);});
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

void CalcWidget::initComboBox() {
    if (m_comboBoxInit)
        return;

    QList<QString> marketsKeys = appData()->prices.markets.keys();
    QList<QString> ratesKeys = appData()->prices.rates.keys();
    if(marketsKeys.count() <= 0 || ratesKeys.count() <= 0) return;

    ui->comboCalcFrom->addItems(marketsKeys);
    ui->comboCalcFrom->insertSeparator(marketsKeys.count());
    ui->comboCalcFrom->addItems(ratesKeys);
    ui->comboCalcFrom->setCurrentIndex(marketsKeys.indexOf("XMR"));

    ui->comboCalcTo->addItems(marketsKeys);
    ui->comboCalcTo->insertSeparator(marketsKeys.count());
    ui->comboCalcTo->addItems(ratesKeys);

    auto preferredFiat = config()->get(Config::preferredFiatCurrency).toString();
    ui->comboCalcTo->setCurrentText(preferredFiat);

    this->m_comboBoxInit = true;
}

void CalcWidget::skinChanged() {
    ui->imageExchange->setMode(ColorScheme::hasDarkBackground(this));
}

CalcWidget::~CalcWidget() {
    delete ui;
}
