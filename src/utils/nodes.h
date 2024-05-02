// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_NODES_H
#define FEATHER_NODES_H

#include <QTimer>
#include <QRegularExpression>
#include <QHostAddress>

#include "model/NodeModel.h"
#include "utils/Utils.h"
#include "utils/config.h"

enum NodeSource {
    websocket = 0,
    custom
};

class NodeList : public QObject {
Q_OBJECT

public:
    enum Type {
        ws = 0,
        custom
    };
    Q_ENUM(Type)

    bool addNode(const QString &node, NetworkType::Type networkType, NodeList::Type source);
    void setNodes(const QStringList &nodes, NetworkType::Type networkType, NodeList::Type source);
    QStringList getNodes(NetworkType::Type networkType, NodeList::Type source);

private:
    void ensureStructure(QJsonObject &obj, NetworkType::Type networkType);
    QJsonObject getConfigData();
};

struct FeatherNode {
    explicit FeatherNode(QString address = "", int height = 0, int target_height = 0, bool online = false)
            : height(height)
            , target_height(target_height)
            , online(online)
    {
        if (address.isEmpty())
            return;

        address.remove("https://"); // todo: regex
        if (!address.startsWith("http://"))
            address.prepend("http://");

        url = QUrl(address);

        if (!url.isValid())
            return;

        if (url.port() == -1)
            url.setPort(18081);
    };

    int height;
    int target_height;
    bool online = false;
    bool cached = false;
    bool custom = false;
    bool isConnecting = false;
    bool isActive = false;
    QUrl url;

    bool isValid() const {
        return url.isValid();
    }

    bool isLocal() const {
        QString host = url.host();

        if (host == "localhost") {
            return true;
        }

        QHostAddress address(host);

        bool validipv4;
        quint32 ipv4 = address.toIPv4Address(&validipv4);

        if (!validipv4) {
            return false;
        }

        bool local = (
                ((ipv4 & 0xff000000) == 0x0a000000) || /*       10/8 */
                ((ipv4 & 0xff000000) == 0x00000000) || /*        0/8 */
                ((ipv4 & 0xff000000) == 0x7f000000) || /*      127/8 */
                ((ipv4 & 0xffc00000) == 0x64400000) || /*  100.64/10 */
                ((ipv4 & 0xffff0000) == 0xa9fe0000) || /* 169.254/16 */
                ((ipv4 & 0xfff00000) == 0xac100000) || /*  172.16/12 */
                ((ipv4 & 0xffff0000) == 0xc0a80000));  /* 192.168/16 */

        return local;
    }

    bool isOnion() const {
        return url.host().endsWith(".onion");
    }

    bool isI2P() const {
        return url.host().endsWith(".i2p");
    }

    bool isAnonymityNetwork() const {
        return isOnion() || isI2P();
    };

    QString toAddress() const {
        return QString("%1:%2").arg(url.host(), QString::number(url.port()));
    }

    QString toFullAddress() const {
        if (!url.userName().isEmpty() && !url.password().isEmpty())
            return QString("%1:%2@%3:%4").arg(url.userName(), url.password(), url.host(), QString::number(url.port()));

        return toAddress();
    }

    QString toURL() const {
        QUrl withScheme(url);
        withScheme.setScheme("http");

        return withScheme.toString(QUrl::RemoveUserInfo | QUrl::RemovePath);
    }

    bool operator == (const FeatherNode &other) const {
        return this->url == other.url;
    }
};

class Nodes : public QObject {
    Q_OBJECT

public:
    explicit Nodes(QObject *parent, Wallet *wallet);
    ~Nodes() override;

    void loadConfig();
    void allowConnection();
    void updateModels();

    NodeSource source();
    FeatherNode connection();

    QList<FeatherNode> nodes();
    QList<FeatherNode> customNodes();
    QList<FeatherNode> websocketNodes();

    NodeModel *modelWebsocket;
    NodeModel *modelCustom;

public slots:
    void connectToNode();
    void connectToNode(const FeatherNode &node);
    void onWSNodesReceived(QList<FeatherNode>& nodes);
    void onNodeSourceChanged(NodeSource nodeSource);
    void setCustomNodes(const QList<FeatherNode>& nodes);
    void autoConnect(bool forceReconnect = false);

private slots:
    void onWalletRefreshed();

private:
    Wallet *m_wallet = nullptr;
    QJsonObject m_configJson;

    NodeList m_nodes;

    QStringList m_recentFailures;

    QList<FeatherNode> m_customNodes;
    QList<FeatherNode> m_websocketNodes;

    FeatherNode m_connection;  // current active connection, if any

    bool m_wsNodesReceived = false;
    bool m_enableAutoconnect = true;

    bool m_allowConnection = false;

    FeatherNode pickEligibleNode();

    bool useOnionNodes();
    bool useI2PNodes();
    bool useSocks5Proxy(const FeatherNode &node);

    void resetLocalState();
    void exhausted();
    int modeHeight(const QList<FeatherNode> &nodes);
};

#endif //FEATHER_NODES_H
