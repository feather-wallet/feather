// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QList>

#include "calcwidget.h"
#include "ui_calcwidget.h"
#include "mainwindow.h"
#include "components.h"
#include "utils/ColorScheme.h"

CalcWidget::CalcWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CalcWidget)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

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

    connect(AppContext::prices, &Prices::fiatPricesUpdated, this, &CalcWidget::initFiat);
    connect(AppContext::prices, &Prices::cryptoPricesUpdated, this, &CalcWidget::initCrypto);
}

void CalcWidget::fromChanged(const QString &data) {
    if(!this->m_comboBoxInit) return;
    if(this->m_changing){
        this->m_changing = false;
        return;
    }

    QString symbolFrom = ui->comboCalcFrom->itemText(ui->comboCalcFrom->currentIndex());
    QString symbolTo = ui->comboCalcTo->itemText(ui->comboCalcTo->currentIndex());

    if(symbolFrom == symbolTo){
        ui->lineTo->setText(data);
        return;
    }

    QString amount_str = ui->lineFrom->text();
    if(amount_str.startsWith('.')){
        ui->lineFrom->setText(ui->lineTo->text());
        return;
    }

    double amount = amount_str.toDouble();
    double result = AppContext::prices->convert(symbolFrom, symbolTo, amount);

    this->m_changing = true;

    int precision = 10;
    if(AppContext::prices->rates.contains(symbolTo))
        precision = 2;

    ui->lineTo->setText(QString::number(result, 'f', precision));
}

void CalcWidget::toChanged(const QString &data) {
    if(!this->m_comboBoxInit) return;
    if(this->m_changing){
        this->m_changing = false;
        return;
    }

    QString symbolFrom = ui->comboCalcFrom->itemText(
            ui->comboCalcFrom->currentIndex());
    QString symbolTo = ui->comboCalcTo->itemText(
            ui->comboCalcTo->currentIndex());

    if(symbolFrom == symbolTo){
        ui->lineTo->setText(ui->lineFrom->text());
        return;
    }

    QString amount_str = ui->lineTo->text();
    if(amount_str.startsWith('.')){
        ui->lineTo->setText("");
        return;
    }

    double amount = amount_str.toDouble();
    double result = AppContext::prices->convert(symbolTo, symbolFrom, amount);

    this->m_changing = true;

    int precision = 10;
    if(AppContext::prices->rates.contains(symbolFrom))
        precision = 2;

    ui->lineFrom->setText(QString::number(result, 'f', precision));
}

void CalcWidget::toComboChanged(const QString &data) {
    this->fromChanged(data);
}

void CalcWidget::initCrypto() {
    this->initComboBox();
}

void CalcWidget::initFiat() {
    this->initComboBox();
}

void CalcWidget::initComboBox() {
    if(m_comboBoxInit) return;
    QList<QString> marketsKeys = AppContext::prices->markets.keys();
    QList<QString> ratesKeys = AppContext::prices->rates.keys();
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
