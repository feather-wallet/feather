// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_NODES_H
#define FEATHER_NODES_H

#include <QTimer>
#include <QRegExp>
#include <QApplication>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "model/NodeModel.h"
#include "utils/utils.h"
#include "utils/config.h"

enum NodeSource {
    websocket = 0,
    custom
};

struct FeatherNode {
    explicit FeatherNode(QString _address = "", int height = 0, int target_height = 0, bool online = false)
            : height(height), target_height(target_height), online(online){
        // wonky ipv4/host parsing, should be fine(tm)(c).
        if(_address.isEmpty()) return;
        if(_address.contains("https://")) {
            this->isHttps = true;
        }
        _address = _address.replace("https://", "");
        _address = _address.replace("http://", "");
        if(_address.contains("@")){  // authentication, user/pass
            const auto spl = _address.split("@");
            const auto &creds = spl.at(0);
            if(creds.contains(":")) {
                const auto _spl = creds.split(":");
                this->username = _spl.at(0).trimmed().replace(" ", "");
                this->password = _spl.at(1).trimmed().replace(" ", "");
            }
            _address = spl.at(1);
        }
        if(!_address.contains(":"))
            _address += ":18081";
        this->address = _address;
        if(this->address.contains(".onion"))
            tor = true;
        this->full = this->generateFull();
    };

    QString address;
    QString full;
    int height;
    int target_height;
    bool online = false;
    QString username;
    QString password;
    bool cached = false;
    bool custom = false;
    bool tor = false;
    bool isConnecting = false;
    bool isActive = false;
    bool isHttps = false;

    QString generateFull() {
        QString auth;
        if(!this->username.isEmpty() && !this->password.isEmpty())
            auth = QString("%1:%2@").arg(this->username).arg(this->password);
        return QString("%1%2").arg(auth).arg(this->address);
    }

    QString as_url() {
        return QString("%1://%2/get_info").arg(this->isHttps ? "https": "http",this->full);
    }

    bool operator == (const FeatherNode &other) const {
        return this->full == other.full;
    }
};

class Nodes : public QObject {
    Q_OBJECT

public:
    explicit Nodes(AppContext *ctx, QNetworkAccessManager *networkAccessManager, QObject *parent = nullptr);
    void loadConfig();
    void writeConfig();

    NodeSource source();
    FeatherNode connection();
    QList<FeatherNode> customNodes();

    NodeModel *modelWebsocket;
    NodeModel *modelCustom;

public slots:
    void connectToNode();
    void connectToNode(const FeatherNode &node);
    void onWSNodesReceived(const QList<QSharedPointer<FeatherNode>>& nodes);
    void onNodeSourceChanged(NodeSource nodeSource);
    void setCustomNodes(const QList<FeatherNode>& nodes);
    void autoConnect(bool forceReconnect = false);

signals:
    void WSNodeExhausted();
    void nodeExhausted();
    void updateStatus(const QString &msg);

private:
    AppContext *m_ctx = nullptr;
    NodeSource m_source = NodeSource::websocket;
    QNetworkAccessManager *m_networkAccessManager = nullptr;
    QJsonObject m_configJson;

    QStringList m_recentFailures;

    QList<FeatherNode> m_customNodes;
    QList<FeatherNode> m_websocketNodes;

    FeatherNode m_connection;  // current active connection, if any

    bool m_wsNodesReceived = false;
    bool m_wsExhaustedWarningEmitted = true;
    bool m_customExhaustedWarningEmitted = true;
    bool m_enableAutoconnect = true;

    FeatherNode pickEligibleNode();

    void updateModels();
    void resetLocalState();
    void exhausted();
    void WSNodeExhaustedWarning();
    void nodeExhaustedWarning();
    int modeHeight(const QList<FeatherNode> &nodes);
};

#endif //FEATHER_NODES_H
