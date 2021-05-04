// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "AppData.h"
#include "config.h"
#include "WebsocketNotifier.h"

AppData::AppData(QObject *parent)
    : QObject(parent)
{
    this->initRestoreHeights();

    auto genesis_timestamp = this->restoreHeights[NetworkType::Type::MAINNET]->data.firstKey();
    this->txFiatHistory = new TxFiatHistory(genesis_timestamp, Config::defaultConfigDir().path());

    connect(&websocketNotifier()->websocketClient, &WebsocketClient::connectionEstablished, this->txFiatHistory, &TxFiatHistory::onUpdateDatabase);
    connect(this->txFiatHistory, &TxFiatHistory::requestYear, [](int year){
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1}})").arg(year).toUtf8();
        websocketNotifier()->websocketClient.sendMsg(data);
    });
    connect(this->txFiatHistory, &TxFiatHistory::requestYearMonth, [](int year, int month){
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1, "month": %2}})").arg(year).arg(month).toUtf8();
        websocketNotifier()->websocketClient.sendMsg(data);
    });

    connect(websocketNotifier(), &WebsocketNotifier::CryptoRatesReceived, &this->prices, &Prices::cryptoPricesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::FiatRatesReceived, &this->prices, &Prices::fiatPricesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::TxFiatHistoryReceived, this->txFiatHistory, &TxFiatHistory::onWSData);
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