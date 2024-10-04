// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "TickerWidget.h"
#include "ui_TickerWidget.h"

#include "constants.h"
#include "utils/AppData.h"
#include "utils/config.h"

TickerWidgetBase::TickerWidgetBase(QWidget *parent, Wallet *wallet)
        : QWidget(parent)
        , ui(new Ui::TickerWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    ui->tickerPct->setFont(Utils::relativeFont(-1));
    ui->tickerFiat->setFont(Utils::relativeFont(0));

    this->setPercentageText("0.0", true);
    ui->tickerFiat->setText("...");
}

TickerWidgetBase::~TickerWidgetBase() = default;

void TickerWidgetBase::setTitle(const QString &title) {
    ui->tickerBox->setTitle(title);
}

void TickerWidgetBase::setPercentageVisible(bool visible) {
    ui->tickerPct->setVisible(visible);
}

void TickerWidgetBase::setPercentageText(const QString &text, bool positive) {
    QString pctText = "<html><head/><body><p><span style=\" color:red;\">";
    if(positive) {
        pctText = pctText.replace("red", "green");
        pctText += QString("+%1%").arg(text);
    } else
        pctText += QString("%1%").arg(text);
    pctText += "</span></p></body></html>";

    ui->tickerPct->setText(pctText);
}

void TickerWidgetBase::setFiatText(double amount, const QString &fiatCurrency) {
    QString conversionText = Utils::amountToCurrencyString(amount, fiatCurrency);
    ui->tickerFiat->setText(conversionText);
}

void TickerWidgetBase::setDisplayText(const QString &text) {
    ui->tickerFiat->setText(text);
}

// BalanceTickerWidget
BalanceTickerWidget::BalanceTickerWidget(QWidget *parent, Wallet *wallet, bool totalBalance)
        : TickerWidgetBase(parent, wallet)
        , m_totalBalance(totalBalance)
{
    if (totalBalance)
        this->setTitle("Total balance");
    else
        this->setTitle("Balance");

    this->setPercentageVisible(false);

    connect(m_wallet, &Wallet::balanceUpdated, this, &BalanceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &BalanceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &BalanceTickerWidget::updateDisplay);
}

void BalanceTickerWidget::updateDisplay() {
    double balance = (m_totalBalance ? m_wallet->balanceAll() : m_wallet->balance()) / constants::cdiv;
    QString fiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
    double balanceFiatAmount = appData()->prices.convert("XMR", fiatCurrency, balance);
    if (balanceFiatAmount < 0)
        return;
    this->setFiatText(balanceFiatAmount, fiatCurrency);
}

// PriceTickerWidget
PriceTickerWidget::PriceTickerWidget(QWidget *parent, Wallet *wallet, QString symbol)
        : TickerWidgetBase(parent, wallet)
        , m_symbol(std::move(symbol))
{
    this->setTitle(m_symbol);

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &PriceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &PriceTickerWidget::updateDisplay);
}

void PriceTickerWidget::updateDisplay() {
    QString fiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
    double price = appData()->prices.convert(m_symbol, fiatCurrency, 1.0);
    if (price < 0)
        return;

    auto markets = appData()->prices.markets;
    if (!markets.contains(m_symbol))
        return;

    double percentChange24h = markets[m_symbol].price_usd_change_pct_24h;
    QString percentChange24hStr = QString::number(percentChange24h, 'f', 1);
    this->setPercentageText(percentChange24hStr, percentChange24h >= 0.0);

    this->setFiatText(price, fiatCurrency);
}

//RatioTickerWidget
RatioTickerWidget::RatioTickerWidget(QWidget *parent, Wallet *wallet, QString symbol1, QString symbol2)
        : TickerWidgetBase(parent, wallet)
        , m_symbol1(std::move(symbol1))
        , m_symbol2(std::move(symbol2))
{
    this->setTitle(QString("%1/%2").arg(m_symbol1, m_symbol2));

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &RatioTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &RatioTickerWidget::updateDisplay);
}

void RatioTickerWidget::updateDisplay() {
    double ratio = appData()->prices.convert(m_symbol1, m_symbol2, 1);
    if (ratio < 0)
        return;

    // Approximation based on USD price
    if (appData()->prices.markets.contains(m_symbol1) && appData()->prices.markets.contains(m_symbol2)) {
        double price_symbol1 = appData()->prices.markets[m_symbol1].price_usd;
        double price_symbol1_24h_ago = price_symbol1 - (price_symbol1 * (appData()->prices.markets[m_symbol1].price_usd_change_pct_24h / 100));

        double price_symbol2 = appData()->prices.markets[m_symbol2].price_usd;
        double price_symbol2_24h_ago = price_symbol2 - (price_symbol2 * (appData()->prices.markets[m_symbol2].price_usd_change_pct_24h / 100));
        if (price_symbol2_24h_ago == 0) return;

        double ratio_24h_ago = price_symbol1_24h_ago / price_symbol2_24h_ago;
        if (ratio_24h_ago == 0) return;

        double percentage_change = ((ratio - ratio_24h_ago) / ratio_24h_ago) * 100;

        this->setPercentageText(QString::number(percentage_change, 'f', 1), ratio >= ratio_24h_ago);
    }

    this->setDisplayText(QString::number(ratio, 'f', 6));
}