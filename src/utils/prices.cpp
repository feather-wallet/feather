// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "utils/prices.h"

Prices::Prices(QObject *parent) : QObject(parent) {
    this->rates = QMap<QString, double>();
    this->markets = QMap<QString, marketStruct>();
}

void Prices::cryptoPricesReceived(const QJsonArray &data) {
    this->markets.clear();

    for (const auto &entry : data) {
        QJsonObject obj = entry.toObject();
        marketStruct ms;
        ms.symbol = obj.value("symbol").toString();
        ms.image = obj.value("image").toString();
        ms.name = obj.value("name").toString();
        ms.price_usd = obj.value("current_price").toDouble();
        ms.price_usd_change_pct_24h = obj.value("price_change_percentage_24h").toDouble();
        if (ms.price_usd <= 0)
            continue;

        this->markets.insert(ms.symbol.toUpper(), ms);
    }

    emit cryptoPricesUpdated();
}

void Prices::fiatPricesReceived(const QJsonObject &data) {
    QJsonObject ratesData = data.value("rates").toObject();
    for (const auto &currency : ratesData) {
        QString currencyStr = currency.toString();
        this->rates.insert(currencyStr, ratesData.value(currencyStr).toDouble());
    }
    emit fiatPricesUpdated();
}

double Prices::convert(QString symbolFrom, QString symbolTo, double amount) {
    if (symbolFrom == symbolTo)
        return amount;
    if (amount <= 0.0)
        return 0.0;

    symbolFrom = symbolFrom.toUpper();
    symbolTo = symbolTo.toUpper();

    double usdPrice;
    if (this->markets.contains(symbolFrom)) {
        usdPrice = this->markets[symbolFrom].price_usd * amount;
    }
    else if (this->rates.contains(symbolFrom)) {
        if (symbolFrom == "USD") {
            usdPrice = amount;
        } else {
            usdPrice = amount / this->rates[symbolFrom];
        }
    }
    else {
        return 0.0;
    }

    if (symbolTo == "USD")
        return usdPrice;

    if (this->markets.contains(symbolTo))
        return usdPrice / this->markets[symbolTo].price_usd;
    else if (this->rates.contains(symbolTo))
        return usdPrice * this->rates[symbolTo];

    return 0.0;
}

