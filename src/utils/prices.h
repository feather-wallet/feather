// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_PRICES_H
#define FEATHER_PRICES_H

#include <QObject>

#include "utils/Utils.h"

struct marketStruct {
    QString symbol;
    QString name;
    QString image;
    double price_usd;
    double price_usd_change_pct_24h;
};

class Prices : public QObject
{
Q_OBJECT

public:
    explicit Prices(QObject *parent = nullptr);
    QMap<QString, double> rates;
    QMap<QString, marketStruct> markets;

public slots:
    void cryptoPricesReceived(const QJsonArray &data);
    void fiatPricesReceived(const QJsonObject &data);

    double convert(QString symbolFrom, QString symbolTo, double amount);

signals:
    void fiatPricesUpdated();
    void cryptoPricesUpdated();
};

#endif //FEATHER_PRICES_H
