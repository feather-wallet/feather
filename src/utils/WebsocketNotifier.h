// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WEBSOCKETNOTIFIER_H
#define FEATHER_WEBSOCKETNOTIFIER_H

#include <QObject>
#include <QMap>

#include "WebsocketClient.h"
#include "networktype.h"
#include "nodes.h"
#include "prices.h"
#include "TxFiatHistory.h"

class WebsocketNotifier : public QObject {
    Q_OBJECT

public:
    explicit WebsocketNotifier(QObject *parent);
    ~WebsocketNotifier();

    QMap<NetworkType::Type, int> heights;
    WebsocketClient *websocketClient;

    static WebsocketNotifier* instance();
    void emitCache();

    bool stale(int minutes);

signals:
    void BlockHeightsReceived(int mainnet, int stagenet);
    void NodesReceived(QList<FeatherNode> &L);
    void CryptoRatesReceived(const QJsonArray &data);
    void FiatRatesReceived(const QJsonObject &fiat_rates);
    void TxFiatHistoryReceived(const QJsonObject &data);
    void UpdatesReceived(const QJsonObject &updates);
    void dataReceived(const QString &type, const QJsonValue &json);

private slots:
    void onWSMessage(const QJsonObject &msg);

    void onWSNodes(const QJsonArray &nodes);
    void onWSUpdates(const QJsonObject &updates);

private:
    static QPointer<WebsocketNotifier> m_instance;

    QStringList m_pluginSubscriptions;
    QHash<QString, QJsonObject> m_cache;
    QDateTime m_lastMessageReceived;
};

inline WebsocketNotifier* websocketNotifier()
{
    return WebsocketNotifier::instance();
}


#endif //FEATHER_WEBSOCKETNOTIFIER_H