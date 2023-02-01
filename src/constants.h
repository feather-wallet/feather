// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_CONSTANTS_H
#define FEATHER_CONSTANTS_H

#include <QtGlobal>
#include <QUrl>

#include "networktype.h"

namespace constants
{
    extern NetworkType::Type networkType; // TODO: not really a const

    // coin constants
    const std::string coinName = "monero";
    const qreal cdiv = 1e12;
    const quint32 mixin = 10;
    const quint64 kdfRounds = 1;

    const QString seedLanguage = "English"; // todo: move me

    // donation constants
    const QString donationAddress = "47ntfT2Z5384zku39pTM6hGcnLnvpRYW2Azm87GiAAH2bcTidtq278TL6HmwyL8yjMeERqGEBs3cqC8vvHPJd1cWQrGC65f";
    const int donationBoundary = 25;

    // websocket constants
    const QVector<QUrl> websocketUrls = {
        QUrl(QStringLiteral("wss://ws.featherwallet.org/ws")),
        QUrl(QStringLiteral("wss://ws.featherwallet.net/ws"))
    };

    // website constants
    const QString websiteUrl = "https://featherwallet.org";
}

#endif //FEATHER_CONSTANTS_H
