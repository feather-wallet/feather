// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "TickerWidget.h"
#include "ui_TickerWidget.h"

#include "constants.h"
#include "utils/AppData.h"

TickerWidgetBase::TickerWidgetBase(QWidget *parent, QSharedPointer<AppContext> ctx)
        : QWidget(parent)
        , ui(new Ui::TickerWidget)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    ui->tickerPct->setFont(Utils::relativeFont(-2));
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

// BalanceTickerWidget
BalanceTickerWidget::BalanceTickerWidget(QWidget *parent, QSharedPointer<AppContext> ctx, bool totalBalance)
        : TickerWidgetBase(parent, std::move(ctx))
        , m_totalBalance(totalBalance)
{
    if (totalBalance)
        this->setTitle("Total balance");
    else
        this->setTitle("Balance");

    this->setPercentageVisible(false);

    connect(m_ctx.get(), &AppContext::balanceUpdated, this, &BalanceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &BalanceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &BalanceTickerWidget::updateDisplay);
}

void BalanceTickerWidget::updateDisplay() {
    double balance = (m_totalBalance ? m_ctx->wallet->balanceAll() : m_ctx->wallet->balance()) / constants::cdiv;
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    double balanceFiatAmount = appData()->prices.convert("XMR", fiatCurrency, balance);
    if (balanceFiatAmount < 0)
        return;
    this->setFiatText(balanceFiatAmount, fiatCurrency);
}

// PriceTickerWidget
PriceTickerWidget::PriceTickerWidget(QWidget *parent, QSharedPointer<AppContext> ctx, QString symbol)
        : TickerWidgetBase(parent, std::move(ctx))
        , m_symbol(std::move(symbol))
{
    this->setTitle(m_symbol);

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &PriceTickerWidget::updateDisplay);
    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &PriceTickerWidget::updateDisplay);
}

void PriceTickerWidget::updateDisplay() {
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    double price = appData()->prices.convert(m_symbol, fiatCurrency, 1.0);
    if (price < 0)
        return;

    auto markets = appData()->prices.markets;
    if (!markets.contains(m_symbol))
        return;

    double percentChange24h = markets[m_symbol].price_usd_change_pct_24h;
    QString percentChange24hStr = QString::number(percentChange24h, 'f', 2);
    this->setPercentageText(percentChange24hStr, percentChange24h >= 0.0);

    this->setFiatText(price, fiatCurrency);
}
