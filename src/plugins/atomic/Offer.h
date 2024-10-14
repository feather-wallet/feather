// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_OFFER_H
#define FEATHER_OFFER_H

#include <QString>

struct OfferEntry {
    OfferEntry(double price, double min, double max, QString address )
            :  price(price), min(min), max(max), address(std::move(address)) {};

    double price;
    double min;
    double max;
    QString address;
};

#endif //FEATHER_OFFER_H