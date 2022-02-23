// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_WEBSOCKETNOTIFIER_H
#define FEATHER_WEBSOCKETNOTIFIER_H

#include <QObject>
#include <QMap>

#include "WebsocketClient.h"
#include "networktype.h"
#include "nodes.h"
#include "prices.h"
#include "widgets/RedditPost.h"
#include "widgets/CCSEntry.h"
#include "TxFiatHistory.h"

class WebsocketNotifier : public QObject {
    Q_OBJECT

public:
    explicit WebsocketNotifier(QObject *parent);

    QMap<NetworkType::Type, int> heights;
    WebsocketClient websocketClient;

    static WebsocketNotifier* instance();
    void emitCache();

    bool stale(int minutes);

signals:
    void BlockHeightsReceived(int mainnet, int stagenet);
    void NodesReceived(QList<FeatherNode> &L);
    void CryptoRatesReceived(const QJsonArray &data);
    void FiatRatesReceived(const QJsonObject &fiat_rates);
    void RedditReceived(QList<QSharedPointer<RedditPost>> L);
    void CCSReceived(QList<QSharedPointer<CCSEntry>> L);
    void TxFiatHistoryReceived(const QJsonObject &data);
    void UpdatesReceived(const QJsonObject &updates);
    void XMRigDownloadsReceived(const QJsonObject &downloads);
    void LocalMoneroCountriesReceived(const QJsonArray &countries);
    void LocalMoneroCurrenciesReceived(const QJsonArray &currencies);
    void LocalMoneroPaymentMethodsReceived(const QJsonObject &payment_methods);

private slots:
    void onWSMessage(const QJsonObject &msg);

    void onWSNodes(const QJsonArray &nodes);
    void onWSReddit(const QJsonArray &reddit_data);
    void onWSCCS(const QJsonArray &ccs_data);
    void onWSUpdates(const QJsonObject &updates);
    void onWSXMRigDownloads(const QJsonObject &downloads);

private:
    static QPointer<WebsocketNotifier> m_instance;

    QHash<QString, QJsonObject> m_cache;
    QDateTime m_lastMessageReceived;
};

inline WebsocketNotifier* websocketNotifier()
{
    return WebsocketNotifier::instance();
}


#endif //FEATHER_WEBSOCKETNOTIFIER_H