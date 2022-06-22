// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "WebsocketNotifier.h"
#include "Utils.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"

#include <QJsonObject>

WebsocketNotifier::WebsocketNotifier(QObject *parent)
    : QObject(parent)
    , websocketClient(new WebsocketClient(this))
{
    connect(&websocketClient, &WebsocketClient::WSMessage, this, &WebsocketNotifier::onWSMessage);
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

    else if(cmd == "reddit") {
        QJsonArray reddit_data = msg.value("data").toArray();
        this->onWSReddit(reddit_data);
    }

    else if(cmd == "ccs") {
        auto ccs_data = msg.value("data").toArray();
        this->onWSCCS(ccs_data);
    }

    else if(cmd == "txFiatHistory") {
        auto txFiatHistory_data = msg.value("data").toObject();
        emit TxFiatHistoryReceived(txFiatHistory_data);
    }

    else if(cmd == "revuo") {
        auto revuo_data = msg.value("data").toArray();
        this->onWSRevuo(revuo_data);
    }

#if defined(CHECK_UPDATES)
    else if (cmd == "updates") {
        this->onWSUpdates(msg.value("data").toObject());
    }
#endif

#if defined(HAS_XMRIG)
    else if(cmd == "xmrig") {
        this->onWSXMRigDownloads(msg.value("data").toObject());
    }
#endif

#if defined(HAS_LOCALMONERO)
    else if (cmd == "localmonero_countries") {
        emit LocalMoneroCountriesReceived(msg.value("data").toArray());
    }

    else if (cmd == "localmonero_currencies") {
        emit LocalMoneroCurrenciesReceived(msg.value("data").toArray());
    }

    else if (cmd == "localmonero_payment_methods") {
        emit LocalMoneroPaymentMethodsReceived(msg.value("data").toObject());
    }
#endif
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

void WebsocketNotifier::onWSReddit(const QJsonArray& reddit_data) {
    QList<QSharedPointer<RedditPost>> l;

    for (auto &&entry: reddit_data) {
        auto obj = entry.toObject();
        auto redditPost = new RedditPost(
                obj.value("title").toString(),
                obj.value("author").toString(),
                obj.value("permalink").toString(),
                obj.value("comments").toInt());
        QSharedPointer<RedditPost> r = QSharedPointer<RedditPost>(redditPost);
        l.append(r);
    }

    emit RedditReceived(l);
}

void WebsocketNotifier::onWSCCS(const QJsonArray &ccs_data) {
    QList<QSharedPointer<CCSEntry>> l;

    QStringList fonts = {"state", "address", "author", "date",
                         "title", "target_amount", "raised_amount",
                         "percentage_funded", "contributions"};

    for (auto &&entry: ccs_data) {
        auto obj = entry.toObject();
        auto c = QSharedPointer<CCSEntry>(new CCSEntry());

        if (obj.value("state").toString() != "FUNDING-REQUIRED")
            continue;

        c->state = obj.value("state").toString();
        c->address = obj.value("address").toString();
        c->author = obj.value("author").toString();
        c->date = obj.value("date").toString();
        c->title = obj.value("title").toString();
        c->url = obj.value("url").toString();
        c->target_amount = obj.value("target_amount").toDouble();
        c->raised_amount = obj.value("raised_amount").toDouble();
        c->percentage_funded = obj.value("percentage_funded").toDouble();
        c->contributions = obj.value("contributions").toInt();
        l.append(c);
    }

    emit CCSReceived(l);
}

void WebsocketNotifier::onWSRevuo(const QJsonArray &revuo_data) {
    QList<QSharedPointer<RevuoItem>> l;

    for (auto &&entry: revuo_data) {
        auto obj = entry.toObject();

        QStringList newsbytes;
        for (const auto &n : obj.value("newsbytes").toArray()) {
            newsbytes.append(n.toString());
        }

        auto revuoItem = new RevuoItem(
                obj.value("title").toString(),
                obj.value("url").toString(),
                newsbytes);

        QSharedPointer<RevuoItem> r = QSharedPointer<RevuoItem>(revuoItem);
        l.append(r);
    }

    emit RevuoReceived(l);
}

void WebsocketNotifier::onWSUpdates(const QJsonObject &updates) {
    emit UpdatesReceived(updates);
}

void WebsocketNotifier::onWSXMRigDownloads(const QJsonObject &downloads) {
    emit XMRigDownloadsReceived(downloads);
}

WebsocketNotifier* WebsocketNotifier::instance()
{
    if (!m_instance) {
        m_instance = new WebsocketNotifier(QCoreApplication::instance());
    }

    return m_instance;
}