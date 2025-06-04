// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AppData.h"

#include <QCoreApplication>

#include "config.h"

AppData::AppData(QObject *parent)
    : QObject(parent)
{
    this->initRestoreHeights();

    auto genesis_timestamp = this->restoreHeights[NetworkType::Type::MAINNET]->data.firstKey();
    this->txFiatHistory = new TxFiatHistory(genesis_timestamp, Config::defaultConfigDir().path(), this);
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
