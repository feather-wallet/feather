// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include <QObject>

#include "nodes.h"
#include "utils/Utils.h"
#include "utils/os/tails.h"
#include "appcontext.h"
#include "constants.h"
#include "utils/WebsocketNotifier.h"
#include "utils/TorManager.h"

bool NodeList::addNode(const QString &node, NetworkType::Type networkType, NodeList::Type source) {
    // We can't obtain references to QJsonObjects...
    QJsonObject obj = this->getConfigData();
    this->ensureStructure(obj, networkType);

    QString networkTypeStr = QString::number(networkType);
    QJsonObject netTypeObj = obj.value(networkTypeStr).toObject();

    QString sourceStr = Utils::QtEnumToString(source);
    QJsonArray sourceArray = netTypeObj.value(sourceStr).toArray();

    if (sourceArray.contains(node)) {
        return false;
    }

    sourceArray.append(node);

    netTypeObj[sourceStr] = sourceArray;
    obj[networkTypeStr] = netTypeObj;

    config()->set(Config::nodes, obj);
    return true;
}

void NodeList::setNodes(const QStringList &nodes, NetworkType::Type networkType, NodeList::Type source) {
    QJsonObject obj = this->getConfigData();
    this->ensureStructure(obj, networkType);

    QString networkTypeStr = QString::number(networkType);
    QJsonObject netTypeObj = obj.value(networkTypeStr).toObject();

    QString sourceStr = Utils::QtEnumToString(source);
    QJsonArray sourceArray = QJsonArray::fromStringList(nodes);

    netTypeObj[sourceStr] = sourceArray;
    obj[networkTypeStr] = netTypeObj;

    config()->set(Config::nodes, obj);
}

QStringList NodeList::getNodes(NetworkType::Type networkType, NodeList::Type source) {
    QJsonObject obj = this->getConfigData();
    this->ensureStructure(obj, networkType);

    QString networkTypeStr = QString::number(networkType);
    QJsonObject netTypeObj = obj.value(networkTypeStr).toObject();

    QString sourceStr = Utils::QtEnumToString(source);
    QJsonArray sourceArray = netTypeObj.value(sourceStr).toArray();

    QStringList nodes;
    for (const auto &node : sourceArray) {
        nodes << node.toString();
    }
    return nodes;
}

QJsonObject NodeList::getConfigData() {
    QJsonObject obj = config()->get(Config::nodes).toJsonObject();

    // Load old config format
    if (obj.isEmpty()) {
        auto jsonData = config()->get(Config::nodes).toByteArray();
        if (Utils::validateJSON(jsonData)) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonData);
            obj = doc.object();
        }
    }

    return obj;
}

void NodeList::ensureStructure(QJsonObject &obj, NetworkType::Type networkType) {
    QString networkTypeStr = QString::number(networkType);
    QJsonObject netTypeObj = obj.value(networkTypeStr).toObject();
    if (!netTypeObj.contains("ws"))
        netTypeObj["ws"] = QJsonArray();
    if (!netTypeObj.contains("custom"))
        netTypeObj["custom"] = QJsonArray();

    obj[networkTypeStr] = netTypeObj;
}

Nodes::Nodes(AppContext *ctx, QObject *parent)
    : QObject(parent)
    , modelWebsocket(new NodeModel(NodeSource::websocket, this))
    , modelCustom(new NodeModel(NodeSource::custom, this))
    , m_ctx(ctx)
    , m_connection(FeatherNode())
{
    this->loadConfig();
    connect(m_ctx, &AppContext::walletRefreshed, this, &Nodes::onWalletRefreshed);
    connect(websocketNotifier(), &WebsocketNotifier::NodesReceived, this, &Nodes::onWSNodesReceived);
}

void Nodes::loadConfig() {
    QStringList customNodes = m_nodes.getNodes(constants::networkType, NodeList::custom);
    for (const auto &node : customNodes) {
        FeatherNode customNode{node};
        customNode.custom = true;

        if (m_connection == customNode) {
            if (m_connection.isActive)
                customNode.isActive = true;
            else if (m_connection.isConnecting)
                customNode.isConnecting = true;
        }

        m_customNodes.append(customNode);
    }

    QStringList websocketNodes = m_nodes.getNodes(constants::networkType, NodeList::ws);
    for (const auto &node : websocketNodes) {
        FeatherNode wsNode{node};
        wsNode.custom = false;
        wsNode.online = true; // assume online

        if (m_connection == wsNode) {
            if (m_connection.isActive)
                wsNode.isActive = true;
            else if (m_connection.isConnecting)
                wsNode.isConnecting = true;
        }

        m_websocketNodes.append(wsNode);
    }

    if (m_websocketNodes.count() > 0) {
        qDebug() << QString("Loaded %1 cached websocket nodes from config").arg(m_websocketNodes.count());
    }

    if (m_customNodes.count() > 0) {
        qDebug() << QString("Loaded %1 custom nodes from config").arg(m_customNodes.count());
    }

    // No nodes cached, fallback to hardcorded list
    if (m_websocketNodes.count() == 0) {
        QByteArray file = Utils::fileOpenQRC(":/assets/nodes.json");
        QJsonDocument nodes_json = QJsonDocument::fromJson(file);
        QJsonObject nodes_obj = nodes_json.object();

        QString netKey;
        if (constants::networkType == NetworkType::MAINNET) {
            netKey = "mainnet";
        } else if (constants::networkType == NetworkType::STAGENET) {
            netKey = "stagenet";
        }

        if (nodes_obj.contains(netKey)) {
            QJsonArray nodes_list;
            nodes_list = nodes_json[netKey].toObject()["tor"].toArray();
            for (const auto &node : nodes_json[netKey].toObject()["clearnet"].toArray()) {
                nodes_list.append(node);
            }

            for (auto node: nodes_list) {
                FeatherNode wsNode(node.toString());
                wsNode.custom = false;
                wsNode.online = true;
                m_websocketNodes.append(wsNode);
                m_nodes.addNode(node.toString(), constants::networkType, NodeList::Type::ws);
            }
        }

        qDebug() << QString("Loaded %1 nodes from hardcoded list").arg(m_websocketNodes.count());
    }

    this->updateModels();
}

void Nodes::connectToNode() {
    // auto connect
    m_wsExhaustedWarningEmitted = false;
    m_customExhaustedWarningEmitted = false;
    this->autoConnect(true);
}

void Nodes::connectToNode(const FeatherNode &node) {
    if (!node.isValid())
        return;

    if (config()->get(Config::offlineMode).toBool()) {
        return;
    }

    qInfo() << QString("Attempting to connect to %1 (%2)").arg(node.toAddress()).arg(node.custom ? "custom" : "ws");

    if (!node.url.userName().isEmpty() && !node.url.password().isEmpty())
        m_ctx->wallet->setDaemonLogin(node.url.userName(), node.url.password());

    // Don't use SSL over Tor
    m_ctx->wallet->setUseSSL(!node.isOnion());

    QString proxyAddress;
    if (useTorProxy(node)) {
        if (!torManager()->isLocalTor()) {
            proxyAddress = QString("%1:%2").arg(torManager()->featherTorHost, QString::number(torManager()->featherTorPort));
        } else {
            proxyAddress = QString("%1:%2").arg(config()->get(Config::socks5Host).toString(),
                                                config()->get(Config::socks5Port).toString());
        }
    }

    m_ctx->wallet->initAsync(node.toAddress(), true, 0, false, false, 0, proxyAddress);

    m_connection = node;
    m_connection.isActive = false;
    m_connection.isConnecting = true;

    this->resetLocalState();
    this->updateModels();
}

void Nodes::autoConnect(bool forceReconnect) {
    // this function is responsible for automatically connecting to a daemon.
    if (m_ctx->wallet == nullptr || !m_enableAutoconnect) {
        return;
    }

    Wallet::ConnectionStatus status = m_ctx->wallet->connectionStatus();
    bool wsMode = (this->source() == NodeSource::websocket);

    if (wsMode && !m_wsNodesReceived && websocketNodes().count() == 0) {
        // this situation should rarely occur due to the usage of the websocket node cache on startup.
        qInfo() << "Feather is in websocket connection mode but was not able to receive any nodes (yet).";
        return;
    }

    if (status == Wallet::ConnectionStatus_Disconnected || forceReconnect) {
        if (m_connection.isValid() && !forceReconnect) {
            m_recentFailures << m_connection.toAddress();
        }

        // try a connect
        FeatherNode node = this->pickEligibleNode();
        this->connectToNode(node);
        return;
    }
    else if ((status == Wallet::ConnectionStatus_Synchronizing || status == Wallet::ConnectionStatus_Synchronized) && m_connection.isConnecting) {
        qInfo() << QString("Node connected to %1").arg(m_connection.toAddress());

        // set current connection object
        m_connection.isConnecting = false;
        m_connection.isActive = true;

        // reset node exhaustion state
        m_wsExhaustedWarningEmitted = false;
        m_customExhaustedWarningEmitted = false;
        m_recentFailures.clear();
    }

    this->resetLocalState();
    this->updateModels();
}

FeatherNode Nodes::pickEligibleNode() {
    // Pick a node at random to connect to
    auto rtn = FeatherNode();
    auto wsMode = (this->source() == NodeSource::websocket);
    auto nodes = wsMode ? websocketNodes() : m_customNodes;

    if (nodes.count() == 0) {
        if (wsMode)
            this->exhausted();
        return rtn;
    }

    QVector<int> node_indeces;
    int i = 0;
    for (const auto &node: nodes) {
        node_indeces.push_back(i);
        i++;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(node_indeces.begin(), node_indeces.end(), std::default_random_engine(seed));

    // Pick random eligible node
    int mode_height = this->modeHeight(nodes);
    for (int index : node_indeces) {
        const FeatherNode &node = nodes.at(index);

        // This may fail to detect bad nodes if cached nodes are used
        // Todo: wait on websocket before connecting, only use cache if websocket is unavailable
        if (wsMode && m_wsNodesReceived) {
            // Ignore offline nodes
            if (!node.online)
                continue;

            // Ignore nodes that are more than 25 blocks behind mode
            if (node.height < (mode_height - 25))
                continue;

            // Ignore nodes that say they aren't synchronized
            if (node.target_height > node.height)
                continue;
        }

        // Don't connect to nodes that failed to connect recently
        if (m_recentFailures.contains(node.toAddress())) {
            continue;
        }

        return node;
    }

    // All nodes tried, and none eligible
    // Don't show node exhaustion warning if single custom node is used
    if (wsMode || node_indeces.size() > 1) {
        this->exhausted();
    }
    return rtn;
}

void Nodes::onWSNodesReceived(QList<FeatherNode> &nodes) {
    m_websocketNodes.clear();

    m_wsNodesReceived = true;

    for (auto &node: nodes) {
        if (m_connection == node) {
            if (m_connection.isActive)
                node.isActive = true;
            else if (m_connection.isConnecting)
                node.isConnecting = true;
        }
        m_websocketNodes.push_back(node);
    }

    // cache into config
    QStringList wsNodeList;
    for (const auto &node : m_websocketNodes) {
        wsNodeList << node.toAddress();
    }
    m_nodes.setNodes(wsNodeList, constants::networkType, NodeList::ws);

    this->resetLocalState();
    this->updateModels();
}

void Nodes::onNodeSourceChanged(NodeSource nodeSource) {
    this->resetLocalState();
    this->updateModels();
    this->connectToNode();
}

void Nodes::setCustomNodes(const QList<FeatherNode> &nodes) {
    m_customNodes.clear();

    QStringList nodesList;
    for (auto const &node: nodes) {
        if (nodesList.contains(node.toAddress())) // skip duplicates
            continue;
        nodesList.append(node.toAddress());
        m_customNodes.append(node);
    }

    m_nodes.setNodes(nodesList, constants::networkType, NodeList::Type::custom);

    this->resetLocalState();
    this->updateModels();
}

void Nodes::onWalletRefreshed() {
    if (config()->get(Config::torPrivacyLevel).toInt() == Config::allTorExceptInitSync) {
        // Don't reconnect if we're connected to a local node (traffic will not be routed through Tor)
        if (m_connection.isLocal())
            return;

        // Don't reconnect if we're already connected to an .onion node
        if (m_connection.isOnion())
            return;

        this->autoConnect(true);
    }
}

bool Nodes::useOnionNodes() {
    auto privacyLevel = config()->get(Config::torPrivacyLevel).toInt();
    if (privacyLevel == Config::allTor) {
        return true;
    }

    if (privacyLevel == Config::allTorExceptInitSync) {
        if (m_ctx->refreshed)
            return true;

        if (appData()->heights.contains(constants::networkType)) {
            int initSyncThreshold = config()->get(Config::initSyncThreshold).toInt();
            int networkHeight = appData()->heights[constants::networkType];

            if (m_ctx->wallet->blockChainHeight() > (networkHeight - initSyncThreshold)) {
                return true;
            }
        }
    }

    return false;
}

bool Nodes::useTorProxy(const FeatherNode &node) {
    if (node.isLocal()) {
        return false;
    }

    if (Utils::isTorsocks()) {
        return false;
    }

    if (TailsOS::detect() || WhonixOS::detect()) {
        return true;
    }

    return this->useOnionNodes();
}

void Nodes::updateModels() {
    this->modelCustom->updateNodes(m_customNodes);

    this->modelWebsocket->updateNodes(this->websocketNodes());
}

void Nodes::resetLocalState() {
    auto resetState = [this](QList<FeatherNode> &model){
        for (auto &node: model) {
            node.isConnecting = false;
            node.isActive = false;

            if (node == m_connection) {
                node.isActive = m_connection.isActive;
                node.isConnecting = m_connection.isConnecting;
            }
        }
    };

    resetState(m_customNodes);
    resetState(m_websocketNodes);
}

void Nodes::exhausted() {
    if (this->source() == NodeSource::websocket)
        this->WSNodeExhaustedWarning();
    else
        this->nodeExhaustedWarning();
}

void Nodes::nodeExhaustedWarning(){
    if (m_customExhaustedWarningEmitted)
        return;

    emit nodeExhausted();
    qWarning() << "Could not find an eligible custom node to connect to.";
    m_customExhaustedWarningEmitted = true;
}

void Nodes::WSNodeExhaustedWarning() {
    if (m_wsExhaustedWarningEmitted)
        return;

    emit WSNodeExhausted();
    qWarning() << "Could not find an eligible websocket node to connect to.";
    m_wsExhaustedWarningEmitted = true;
}

QList<FeatherNode> Nodes::nodes() {
    // Return current node list
    return (this->source() == NodeSource::websocket) ? websocketNodes() : m_customNodes;
}

QList<FeatherNode> Nodes::customNodes() {
    return m_customNodes;
}

QList<FeatherNode> Nodes::websocketNodes() {
    bool onionNode = this->useOnionNodes();

    QList<FeatherNode> nodes;
    for (const auto &node : m_websocketNodes) {
        if (onionNode && !node.isOnion()) {
            continue;
        }

        if (!onionNode && node.isOnion()) {
            continue;
        }

        nodes.push_back(node);
    }

    return nodes;
}

void Nodes::onTorSettingsChanged() {
    this->autoConnect(true);
}

FeatherNode Nodes::connection() {
    return m_connection;
}

NodeSource Nodes::source() {
    return static_cast<NodeSource>(config()->get(Config::nodeSource).toInt());
}

int Nodes::modeHeight(const QList<FeatherNode> &nodes) {
    QVector<int> heights;
    for (const auto &node: nodes) {
        heights.push_back(node.height);
    }

    std::sort(heights.begin(), heights.end());

    int max_count = 1, mode_height = heights[0], count = 1;
    for (int i = 1; i < heights.count(); i++) {
        if (heights[i] == 0) { // Don't consider 0 height nodes
            continue;
        }

        if (heights[i] == heights[i - 1])
            count++;
        else {
            if (count > max_count) {
                max_count = count;
                mode_height = heights[i - 1];
            }
            count = 1;
        }
    }
    if (count > max_count)
    {
        mode_height = heights[heights.count() - 1];
    }

    return mode_height;
}