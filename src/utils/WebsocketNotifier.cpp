// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "WebsocketNotifier.h"
#include "Utils.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"
#include "plugins/PluginRegistry.h"

#include <QJsonObject>

WebsocketNotifier::WebsocketNotifier(QObject *parent)
    : QObject(parent)
    , websocketClient(new WebsocketClient(this))
{
    connect(websocketClient, &WebsocketClient::WSMessage, this, &WebsocketNotifier::onWSMessage);

    for (const auto& plugin : PluginRegistry::getPlugins()) {
        m_pluginSubscriptions << plugin->socketData();
    }
}

QPointer<WebsocketNotifier> WebsocketNotifier::m_instance(nullptr);

void WebsocketNotifier::onWSMessage(const QJsonObject &msg) {
    QString cmd = msg.value("cmd").toString();

    m_lastMessageReceived = QDateTime::currentDateTimeUtc();
    m_cache[cmd] = msg;

    if (cmd == "blockheights") {
        QJsonObject data = msg.value("data").toObject();
        int mainnet = data.value("mainnet").toInt();
        int stagenet = data.value("stagenet").toInt();

        emit BlockHeightsReceived(mainnet, stagenet);
    }

    else if(cmd == "nodes") {
        this->onWSNodes(msg.value("data").toArray());
    }

    else if(cmd == "crypto_rates") {
        QJsonArray crypto_rates = msg.value("data").toArray();
        emit CryptoRatesReceived(crypto_rates);
    }

    else if(cmd == "fiat_rates") {
        QJsonObject fiat_rates = msg.value("data").toObject();
        emit FiatRatesReceived(fiat_rates);
    }

    else if(cmd == "txFiatHistory") {
        auto txFiatHistory_data = msg.value("data").toObject();
        emit TxFiatHistoryReceived(txFiatHistory_data);
    }

#if defined(CHECK_UPDATES)
    else if (cmd == "updates") {
        this->onWSUpdates(msg.value("data").toObject());
    }
#endif

    else if (m_pluginSubscriptions.contains(cmd)) {
        emit dataReceived(cmd, msg.value("data"));
    }
}

void WebsocketNotifier::emitCache() {
    for (const auto &msg : m_cache) {
        this->onWSMessage(msg);
    }
}

bool WebsocketNotifier::stale(int minutes) {
    return m_lastMessageReceived < QDateTime::currentDateTimeUtc().addSecs(-(minutes*60));
}

void WebsocketNotifier::onWSNodes(const QJsonArray &nodes) {
    // TODO: Refactor, should be filtered client side

    QList<FeatherNode> l;
    for (auto &&entry: nodes) {
        auto obj = entry.toObject();
        auto nettype = obj.value("nettype");
        auto type = obj.value("type");

        auto networkType = constants::networkType;

        // filter remote node network types
        if(nettype == "mainnet" && networkType != NetworkType::MAINNET)
            continue;
        if(nettype == "stagenet" && networkType != NetworkType::STAGENET)
            continue;
        if(nettype == "testnet" && networkType != NetworkType::TESTNET)
            continue;

        FeatherNode node{obj.value("address").toString(),
                         obj.value("height").toInt(),
                         obj.value("target_height").toInt(),
                         obj.value("online").toBool()};
        l.append(node);
    }

    emit NodesReceived(l);
}

void WebsocketNotifier::onWSUpdates(const QJsonObject &updates) {
    emit UpdatesReceived(updates);
}

WebsocketNotifier* WebsocketNotifier::instance()
{
    if (!m_instance) {
        m_instance = new WebsocketNotifier(QCoreApplication::instance());
    }

    return m_instance;
}