// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AppData.h"

#include <QCoreApplication>

#include "config.h"
#include "WebsocketNotifier.h"

AppData::AppData(QObject *parent)
    : QObject(parent)
{
    this->initRestoreHeights();

    connect(websocketNotifier(), &WebsocketNotifier::CryptoRatesReceived, &this->prices, &Prices::cryptoPricesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::FiatRatesReceived, &this->prices, &Prices::fiatPricesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::BlockHeightsReceived, this, &AppData::onBlockHeightsReceived);
}

QPointer<AppData> AppData::m_instance(nullptr);

void AppData::onBlockHeightsReceived(int mainnet, int stagenet) {
    this->heights[NetworkType::MAINNET] = mainnet;
    this->heights[NetworkType::STAGENET] = stagenet;
}

void AppData::initRestoreHeights() {
    restoreHeights[NetworkType::TESTNET] = new RestoreHeightLookup(NetworkType::TESTNET);
    restoreHeights[NetworkType::STAGENET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_monero_stagenet.txt", NetworkType::STAGENET);
    restoreHeights[NetworkType::MAINNET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_monero_mainnet.txt", NetworkType::MAINNET);
}

AppData* AppData::instance()
{
    if (!m_instance) {
        m_instance = new AppData(QCoreApplication::instance());
    }

    return m_instance;
}
