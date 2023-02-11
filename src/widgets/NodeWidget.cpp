// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "NodeWidget.h"
#include "ui_NodeWidget.h"

#include <QAction>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QTableWidget>

#include "model/NodeModel.h"
#include "utils/Icons.h"

NodeWidget::NodeWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::NodeWidget)
{
    ui->setupUi(this);

    connect(ui->btn_addCustomNodes, &QPushButton::clicked, this, &NodeWidget::onCustomAddClicked);

    connect(ui->checkBox_websocketList, &QCheckBox::stateChanged, [this](int id){
        bool custom = (id == 0);
        ui->stackedWidget->setCurrentIndex(custom);
        ui->frame_addCustomNodes->setVisible(custom);
        config()->set(Config::nodeSource, custom);
        emit nodeSourceChanged(static_cast<NodeSource>(custom));
    });

    m_contextActionRemove = new QAction("Remove", this);
    m_contextActionConnect = new QAction("Connect to node", this);
    m_contextActionOpenStatusURL = new QAction("Visit status page", this);
    m_contextActionCopy = new QAction("Copy", this);
    connect(m_contextActionConnect, &QAction::triggered, this, &NodeWidget::onContextConnect);
    connect(m_contextActionRemove, &QAction::triggered, this, &NodeWidget::onContextCustomNodeRemove);
    connect(m_contextActionOpenStatusURL, &QAction::triggered, this, &NodeWidget::onContextStatusURL);
    connect(m_contextActionCopy, &QAction::triggered, this, &NodeWidget::onContextNodeCopy);
    connect(m_contextActionRemove, &QAction::triggered, this, &NodeWidget::onContextCustomNodeRemove);

    ui->treeView_websocket->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView_custom->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView_websocket, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowWSContextMenu);
    connect(ui->treeView_custom, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowCustomContextMenu);

    connect(ui->treeView_websocket, &QTreeView::doubleClicked, this, &NodeWidget::onContextConnect);
    connect(ui->treeView_custom, &QTreeView::doubleClicked, this, &NodeWidget::onContextConnect);

    int index = config()->get(Config::nodeSource).toInt();
    ui->stackedWidget->setCurrentIndex(config()->get(Config::nodeSource).toInt());
    ui->frame_addCustomNodes->setVisible(index);

    this->onWebsocketStatusChanged();
}

void NodeWidget::onShowWSContextMenu(const QPoint &pos) {
    m_activeView = ui->treeView_websocket;
    FeatherNode node = this->selectedNode();
    if (node.toAddress().isEmpty()) return;

    this->showContextMenu(pos, node);
}

void NodeWidget::onShowCustomContextMenu(const QPoint &pos) {
    m_activeView = ui->treeView_custom;
    FeatherNode node = this->selectedNode();
    if (node.toAddress().isEmpty()) return;

    this->showContextMenu(pos, node);
}

void NodeWidget::onWebsocketStatusChanged() {
    bool disabled = config()->get(Config::disableWebsocket).toBool() || config()->get(Config::offlineMode).toBool();
    ui->treeView_websocket->setColumnHidden(1, disabled);
}

void NodeWidget::showContextMenu(const QPoint &pos, const FeatherNode &node) {
    QMenu menu(this);

    if (m_canConnect && !node.isActive) {
        menu.addAction(m_contextActionConnect);
    }

    menu.addAction(m_contextActionOpenStatusURL);
    menu.addAction(m_contextActionCopy);

    if (m_activeView == ui->treeView_custom) {
        menu.addAction(m_contextActionRemove);
    }

    menu.exec(m_activeView->viewport()->mapToGlobal(pos));
}

void NodeWidget::onContextConnect() {
    QObject *obj = sender();
    if (obj == ui->treeView_custom)
        m_activeView = ui->treeView_custom;
    else
        m_activeView = ui->treeView_websocket;

    FeatherNode node = this->selectedNode();
    if (!node.toAddress().isEmpty())
        emit connectToNode(node);
}

void NodeWidget::onContextStatusURL() {
    FeatherNode node = this->selectedNode();
    if (!node.toAddress().isEmpty())
        Utils::externalLinkWarning(this, QString("%1/get_info").arg(node.toURL()));
}

void NodeWidget::onContextNodeCopy() {
    FeatherNode node = this->selectedNode();
    Utils::copyToClipboard(node.toAddress());
}

FeatherNode NodeWidget::selectedNode() {
    QModelIndex index = m_activeView->currentIndex();
    if (!index.isValid()) return FeatherNode();

    FeatherNode node;
    if (m_activeView == ui->treeView_custom) {
        node = m_customModel->node(index.row());
    } else {
        node = m_wsModel->node(index.row());
    }
    return node;
}

void NodeWidget::onContextCustomNodeRemove() {
    QModelIndex index = ui->treeView_custom->currentIndex();
    if (!index.isValid()) {
        return;
    }
    FeatherNode node = m_customModel->node(index.row());

    auto nodes = m_nodes->customNodes();
    QMutableListIterator<FeatherNode> i(nodes);
    while (i.hasNext())
        if (i.next() == node)
            i.remove();

    m_nodes->setCustomNodes(nodes);
}

void NodeWidget::onCustomAddClicked(){
    auto currentNodes = m_nodes->customNodes();
    QString currentNodesText;

    for (auto &entry: currentNodes) {
        currentNodesText += QString("%1\n").arg(entry.toFullAddress());
    }

    bool ok;
    QString text = QInputDialog::getMultiLineText(this, "Add custom node(s).", "One node per line\nE.g: user:password@127.0.0.1:18081", currentNodesText, &ok);
    if (!ok || text.isEmpty()) {
        return;
    }

    QList<FeatherNode> nodesList;
    auto newNodesList = text.split("\n");
    for (auto &newNodeText: newNodesList) {
        newNodeText = newNodeText.replace("\r", "").trimmed();
        if (newNodeText.isEmpty()) {
            continue;
        }

        auto node = FeatherNode(newNodeText);
        node.custom = true;
        nodesList.append(node);
    }

    m_nodes->setCustomNodes(nodesList);
}

void NodeWidget::setupUI(Nodes *nodes) {
    m_nodes = nodes;
    
    auto nodeSource = m_nodes->source();
    ui->checkBox_websocketList->setChecked(nodeSource == NodeSource::websocket);

    this->setWSModel(m_nodes->modelWebsocket);
    this->setCustomModel(m_nodes->modelCustom);
}

void NodeWidget::setCanConnect(bool canConnect) {
    m_canConnect = canConnect;
}

void NodeWidget::setWSModel(NodeModel *model) {
    m_wsModel = model;
    ui->treeView_websocket->setModel(m_wsModel);
    ui->treeView_websocket->header()->setSectionResizeMode(NodeModel::URL, QHeaderView::Stretch);
    ui->treeView_websocket->header()->setSectionResizeMode(NodeModel::Height, QHeaderView::ResizeToContents);
}

void NodeWidget::setCustomModel(NodeModel *model) {
    m_customModel = model;
    ui->treeView_custom->setModel(m_customModel);
    ui->treeView_custom->header()->setSectionResizeMode(NodeModel::URL, QHeaderView::Stretch);
}

NodeModel* NodeWidget::model() {
    return m_wsModel;
}

NodeWidget::~NodeWidget() = default;
