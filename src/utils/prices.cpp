// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QStandardPaths>
#include <QScreen>
#include <QObject>
#include <QDesktopWidget>

#include "utils/prices.h"

Prices::Prices(QObject *parent) : QObject(parent) {
    this->rates = QMap<QString, double>();
    this->markets = QMap<QString, marketStruct>();
    this->fiat = QMap<QString, QString>();
    fiat["USD"] = "$";
    fiat["EUR"] = "€";
    fiat["JPY"] = "¥";
    fiat["KRW"] = "₩";
    fiat["MXN"] = "$";
    fiat["RUB"] = "\u20BD";
    fiat["CAD"] = "$";
    fiat["GBP"] = "£";
    fiat["SEK"] = "kr";
    fiat["ZAR"] = "R";
    fiat["THB"] = "฿";
    fiat["TRY"] = "₺";
    fiat["CHF"] = "Fr";
    fiat["CNY"] = "¥";
    fiat["CZK"] = "Kč";
    fiat["AUD"] = "$";
    fiat["NZD"] = "$";
}

void Prices::cryptoPricesReceived(const QJsonArray &data) {
    QStringList filter = QStringList() << "XMR" << "ZEC" << "BTC" << "ETH" << "BCH" << "LTC";
    filter << "EOS" << "ADA" << "XLM" << "TRX" << "DASH" << "DCR" << "VET" << "DOGE" << "XRP" << "WOW";

    QMap<QString, marketStruct> msMap;
    for(auto &&entry: data) {
        marketStruct ms;
        QJsonObject obj = entry.toObject();
        ms.symbol = obj.value("symbol").toString();
        ms.image = obj.value("image").toString();
        ms.name = obj.value("name").toString();
        ms.price_usd = obj.value("current_price").toDouble();
        ms.price_usd_change_pct_24h = obj.value("price_change_percentage_24h").toDouble();

        if(ms.price_usd <= 0) continue;
        if(filter.contains(ms.symbol.toUpper()))
            msMap.insert(ms.symbol.toUpper(), ms);
    }

    if(msMap.count() > 0)
        this->markets = msMap;

    emit cryptoPricesUpdated();
}

double Prices::convert(const QString &symbolFrom, const QString &symbolTo, double amount) {
    if(symbolFrom == symbolTo) return amount;
    if(amount <= 0.0) return 0.0;

    double usd_from;
    QString from = symbolFrom.toUpper();
    QString to = symbolTo.toUpper();

    if(this->markets.contains(from))
        usd_from = this->markets[from].price_usd * amount;
    else if(this->rates.contains(from)) {
        if(from == "USD")
            usd_from = amount;
        else
            usd_from = amount / this->rates[from];
    } else
        return 0.0;

    if(to == "USD")
        return usd_from;

    if(this->markets.contains(to))
        return usd_from / this->markets[to].price_usd;
    else if(this->rates.contains(to))
        return usd_from * this->rates[to];

    return 0.0;
}

void Prices::fiatPricesReceived(const QJsonObject &data) {
    QJsonObject rates = data.value("rates").toObject();
    for(const auto &currency: fiat.keys())
        if(rates.contains(currency))
            this->rates.insert(currency, rates.value(currency).toDouble());
    emit fiatPricesUpdated();
}
