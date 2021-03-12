// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_GLOBALS_H
#define FEATHER_GLOBALS_H

#include <QtGlobal>
#include <QUrl>

namespace globals
{
    // coin constants
    const std::string coinName = "monero";
    const qreal cdiv = 1e12;
    const quint32 mixin = 10;
    const quint64 kdfRounds = 1;

    // donation constants
    const QString donationAddress = "47ntfT2Z5384zku39pTM6hGcnLnvpRYW2Azm87GiAAH2bcTidtq278TL6HmwyL8yjMeERqGEBs3cqC8vvHPJd1cWQrGC65f";
    const int donationAmount = 25; // euro
    const int donationBoundary = 15;

    // websocket constants
    const QUrl websocketUrl = QUrl(QStringLiteral("ws://7e6egbawekbkxzkv4244pqeqgoo4axko2imgjbedwnn6s5yb6b7oliqd.onion/ws"));
}

#endif //FEATHER_GLOBALS_H
