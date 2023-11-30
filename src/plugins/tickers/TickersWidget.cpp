// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TickersWidget.h"
#include "ui_TickersWidget.h"

#include "utils/config.h"
#include "WindowManager.h"

TickersWidget::TickersWidget(QWidget *parent, Wallet *wallet)
    : QWidget(parent)
    , ui(new Ui::TickersWidget)
    , m_wallet(wallet)
{
    ui->setupUi(this);
    this->setup();

    // TODO: this is a hack: find a better way to route settings signals to plugins
    connect(windowManager(), &WindowManager::updateBalance, this, &TickersWidget::updateBalance);
    connect(windowManager(), &WindowManager::preferredFiatCurrencyChanged, this, &TickersWidget::updateDisplay);
    connect(windowManager(), &WindowManager::pluginConfigured, [this](const QString &id) {
       if (id == "tickers") {
           this->setup();
       }
    });
    this->updateBalance();
}

void TickersWidget::setup() {
    QStringList tickers = conf()->get(Config::tickers).toStringList();

    Utils::clearLayout(ui->tickerLayout);
    Utils::clearLayout(ui->fiatTickerLayout);

    m_tickerWidgets.clear();
    m_balanceTickerWidget.reset(nullptr);

    for (const auto &ticker : tickers) {
        if (ticker.contains("/")) { // ratio
            QStringList symbols = ticker.split("/");
            if (symbols.length() != 2) {
                qWarning() << "Invalid ticker in config: " << ticker;
            }
            auto* tickerWidget = new RatioTickerWidget(this, m_wallet, symbols[0], symbols[1]);
            m_tickerWidgets.append(tickerWidget);
            ui->tickerLayout->addWidget(tickerWidget);
        } else {
            auto* tickerWidget = new PriceTickerWidget(this, m_wallet, ticker);
            m_tickerWidgets.append(tickerWidget);
            ui->tickerLayout->addWidget(tickerWidget);
        }
    }

    if (conf()->get(Config::tickersShowFiatBalance).toBool()) {
        m_balanceTickerWidget.reset(new BalanceTickerWidget(this, m_wallet, false));
        ui->fiatTickerLayout->addWidget(m_balanceTickerWidget.data());
    }

    this->updateBalance();
    this->updateDisplay();
}

void TickersWidget::updateBalance() {
    ui->frame_fiatTickerLayout->setHidden(conf()->get(Config::hideBalance).toBool());
}

void TickersWidget::updateDisplay() {
    for (const auto &widget : m_tickerWidgets) {
        widget->updateDisplay();
    }
    if (m_balanceTickerWidget) {
        m_balanceTickerWidget->updateDisplay();
    }
}

TickersWidget::~TickersWidget() = default;