// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QtCore>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "nodes.h"
#include "utils/utils.h"
#include "utils/networking.h"
#include "appcontext.h"

Nodes::Nodes(AppContext *ctx, QNetworkAccessManager *networkAccessManager, QObject *parent) :
        QObject(parent),
        m_ctx(ctx),
        m_networkAccessManager(networkAccessManager),
        m_connection(FeatherNode("", 0, false)),
        modelWebsocket(new NodeModel(NodeSource::websocket, this)),
        modelCustom(new NodeModel(NodeSource::custom, this)) {
    this->loadConfig();

    connect(m_connectionTimer, &QTimer::timeout, this, &Nodes::onConnectionTimer);
}

void Nodes::loadConfig() {
    QString msg;
    auto configNodes = config()->get(Config::nodes).toByteArray();
    auto key = QString::number(m_ctx->networkType);
    if (!Utils::validateJSON(configNodes)) {
        m_configJson[key] = QJsonObject();
        qCritical() << "fixed malformed config key \"nodes\"";
    }

    QJsonDocument doc = QJsonDocument::fromJson(configNodes);
    m_configJson = doc.object();

    if(!m_configJson.contains(key))
        m_configJson[key] = QJsonObject();

    auto obj = m_configJson.value(key).toObject();
    if (!obj.contains("custom"))
        obj["custom"] = QJsonArray();
    if (!obj.contains("ws"))
        obj["ws"] = QJsonArray();

    // load custom nodes
    auto nodes = obj.value("custom").toArray();
    foreach (const QJsonValue &value, nodes) {
        auto customNode = FeatherNode(value.toString(), 0, false);
        customNode.custom = true;

        if(m_connection == customNode) {
            if(m_connection.isActive)
                customNode.isActive = true;
            else if(m_connection.isConnecting)
                customNode.isConnecting = true;
        }

        m_customNodes.append(customNode);
    }

    // load cached websocket nodes
    auto ws = obj.value("ws").toArray();
    foreach (const QJsonValue &value, ws) {
        auto wsNode = FeatherNode(value.toString(), 0, false);
        wsNode.custom = false;
        wsNode.online = true;  // assume online

        if(m_connection == wsNode) {
            if(m_connection.isActive)
                wsNode.isActive = true;
            else if(m_connection.isConnecting)
                wsNode.isConnecting = true;
        }

        m_websocketNodes.append(wsNode);
    }

    if (!obj.contains("source"))
        obj["source"] = NodeSource::websocket;
    m_source = static_cast<NodeSource>(obj.value("source").toInt());

    if(m_websocketNodes.count() > 0){
        msg = QString("Loaded %1 cached websocket nodes from config").arg(m_websocketNodes.count());
        activityLog.append(msg);
    }

    if(m_customNodes.count() > 0){
        msg = QString("Loaded %1 custom nodes from config").arg(m_customNodes.count());
        activityLog.append(msg);
    }

    m_configJson[key] = obj;
    this->writeConfig();
    this->updateModels();
}

void Nodes::writeConfig() {
    QJsonDocument doc(m_configJson);
    QString output(doc.toJson(QJsonDocument::Compact));
    config()->set(Config::nodes, output);

    auto msg = QString("Saved node config.");
    activityLog.append(msg);
}

void Nodes::connectToNode() {
    // auto connect
    m_connectionAttempts.clear();
    m_wsExhaustedWarningEmitted = false;
    m_customExhaustedWarningEmitted = false;
    m_connectionTimer->start(2000);
    this->onConnectionTimer();
}

void Nodes::connectToNode(FeatherNode node) {
    if(node.address.isEmpty())
        return;

    emit updateStatus(QString("Connecting to %1").arg(node.address));
    auto msg = QString("Attempting to connect to %1 (%2)")
            .arg(node.address).arg(node.custom ? "custom" : "ws");
    qInfo() << msg;
    activityLog.append(msg);

    if(!node.username.isEmpty() && !node.password.isEmpty())
        m_ctx->currentWallet->setDaemonLogin(node.username, node.password);
    m_ctx->currentWallet->initAsync(node.address, true, 0, false, false, 0);
    m_connectionAttemptTime = std::time(nullptr);

    m_connection = node;
    m_connection.isActive = false;
    m_connection.isConnecting = true;

    this->resetLocalState();
    this->updateModels();

    m_connectionTimer->start(1000);
}

void Nodes::onConnectionTimer() {
    // this function is responsible for automatically connecting to a daemon.
    if (m_ctx->currentWallet == nullptr) {
        m_connectionTimer->stop();
        return;
    }

    QString msg;
    Wallet::ConnectionStatus status = m_ctx->currentWallet->connected(true);
    NodeSource nodeSource = this->source();
    auto wsMode = nodeSource == NodeSource::websocket;
    auto nodes = wsMode ? m_customNodes : m_websocketNodes;

    if(wsMode && !m_wsNodesReceived && m_websocketNodes.count() == 0) {
        // this situation should rarely occur due to the usage of the websocket node cache on startup.
        msg = QString("Feather is in websocket connection mode but was not able to receive any nodes (yet).");
        qInfo() << msg;
        activityLog.append(msg);
        return;
    }

    if(status == Wallet::ConnectionStatus::ConnectionStatus_Disconnected) {
        // try a connect
        auto node = this->pickEligibleNode();
        this->connectToNode(node);
        return;
    } else if(status == Wallet::ConnectionStatus::ConnectionStatus_Connecting){
        if(!m_connection.isConnecting) {
            // Weirdly enough, status == connecting directly after a wallet is opened.
            auto node = this->pickEligibleNode();
            this->connectToNode(node);
            return;
        }

        // determine timeout
        unsigned int nodeConnectionTimeout = 6;
        if(m_connection.tor)
            nodeConnectionTimeout = 25;

        auto connectionTimeout = static_cast<unsigned int>(std::time(nullptr) - m_connectionAttemptTime);
        if(connectionTimeout < nodeConnectionTimeout)
            return; // timeout not reached yet

        msg = QString("Node connection attempt stale after %1 seconds, picking new node").arg(nodeConnectionTimeout);
        activityLog.append(msg);
        qInfo() << msg;

        auto newNode = this->pickEligibleNode();
        this->connectToNode(newNode);
        return;
    } else if(status == Wallet::ConnectionStatus::ConnectionStatus_Connected) {
        // wallet is connected to daemon successfully, poll status every 3 seconds
        if(!m_connection.isConnecting)
            return;

        msg = QString("Node connected to %1").arg(m_connection.address);
        qInfo() << msg;
        activityLog.append(msg);

        // set current connection object
        m_connection.isConnecting = false;
        m_connection.isActive = true;
        this->resetLocalState();
        this->updateModels();

        // reset node exhaustion state
        m_connectionAttempts.clear();
        m_wsExhaustedWarningEmitted = false;
        m_customExhaustedWarningEmitted = false;
        m_connectionTimer->setInterval(3000);
    }
}

FeatherNode Nodes::pickEligibleNode() {
    // Pick a node at random to connect to
    FeatherNode rtn("", 0, false);
    NodeSource nodeSource = this->source();
    auto wsMode = nodeSource == NodeSource::websocket;
    auto nodes = wsMode ? m_websocketNodes : m_customNodes;

    if(nodes.count() == 0) {
        this->exhausted();
        return rtn;
    }

    while(true) {
        // keep track of nodes we have previously tried to connect to
        if(m_connectionAttempts.count() == nodes.count()) {
            this->exhausted();
            m_connectionTimer->stop();
            return rtn;
        }

        int random = QRandomGenerator::global()->bounded(nodes.count());
        FeatherNode node = nodes.at(random);
        if(m_connectionAttempts.contains(node.full))
            continue;
        m_connectionAttempts.append(node.full);

        if(wsMode && !node.online)
            continue;
        return node;
    }
}

void Nodes::onWSNodesReceived(const QList<QSharedPointer<FeatherNode>> &nodes) {
    m_websocketNodes.clear();
    m_wsNodesReceived = true;

    for(auto &node: nodes) {
        if(m_connection == *node) {
            if(m_connection.isActive)
                node->isActive = true;
            else if(m_connection.isConnecting)
                node->isConnecting = true;
        }
        m_websocketNodes.push_back(*node);
    }

    // cache into config
    auto key = QString::number(m_ctx->networkType);
    auto obj = m_configJson.value(key).toObject();
    auto ws = QJsonArray();
    for(auto const &node: m_websocketNodes)
        ws.push_back(node.address);

    obj["ws"] = ws;
    m_configJson[key] = obj;
    this->writeConfig();
    this->resetLocalState();
    this->updateModels();
}

void Nodes::onNodeSourceChanged(NodeSource nodeSource) {
    if(nodeSource == this->source()) return;
    auto key = QString::number(m_ctx->networkType);
    auto obj = m_configJson.value(key).toObject();
    obj["source"] = nodeSource;

    m_configJson[key] = obj;
    this->writeConfig();
    this->resetLocalState();
    this->updateModels();
}

void Nodes::setCustomNodes(QList<FeatherNode> nodes) {
    m_customNodes.clear();
    auto key = QString::number(m_ctx->networkType);
    auto obj = m_configJson.value(key).toObject();

    QStringList nodesList;
    for(auto const &node: nodes) {
        if(nodesList.contains(node.full)) continue;
        nodesList.append(node.full);
        m_customNodes.append(node);
    }

    auto arr = QJsonArray::fromStringList(nodesList);
    obj["custom"] = arr;
    m_configJson[key] = obj;
    this->writeConfig();
    this->resetLocalState();
    this->updateModels();
}

void Nodes::updateModels() {
    this->modelCustom->updateNodes(m_customNodes);
    this->modelWebsocket->updateNodes(m_websocketNodes);
}

void Nodes::resetLocalState() {
    QList<QList<FeatherNode>*> models = {&m_customNodes, &m_websocketNodes};

    for(QList<FeatherNode> *model: models) {
        for (FeatherNode &_node: *model) {
            _node.isConnecting = false;
            _node.isActive = false;

            if (_node == m_connection) {
                _node.isActive = m_connection.isActive;
                _node.isConnecting = m_connection.isConnecting;
            }
        }
    }
}

void Nodes::exhausted() {
    NodeSource nodeSource = this->source();
    auto wsMode = nodeSource == NodeSource::websocket;
    if(wsMode)
        this->WSNodeExhaustedWarning();
    else
        this->nodeExhaustedWarning();
}

void Nodes::nodeExhaustedWarning(){
    if(m_customExhaustedWarningEmitted) return;
    emit nodeExhausted();

    auto msg = QString("Could not find an eligible custom node to connect to.");
    qWarning() << msg;
    activityLog.append(msg);

    m_customExhaustedWarningEmitted = true;
    this->m_connectionTimer->stop();
}

void Nodes::WSNodeExhaustedWarning() {
    if(m_wsExhaustedWarningEmitted) return;
    emit WSNodeExhausted();

    auto msg = QString("Could not find an eligible websocket node to connect to.");
    qWarning() << msg;
    activityLog.append(msg);

    m_wsExhaustedWarningEmitted = true;
    this->m_connectionTimer->stop();
}

void Nodes::onWalletClosing() {
    m_connectionTimer->stop();
}

QList<FeatherNode> Nodes::customNodes() {
    return m_customNodes;
}

FeatherNode Nodes::connection() {
    return m_connection;
}

void Nodes::stopTimer(){
    m_connectionTimer->stop();
}

NodeSource Nodes::source() {
    return m_source;
}
