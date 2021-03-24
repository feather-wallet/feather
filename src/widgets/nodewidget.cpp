// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QInputDialog>
#include <QTableWidget>
#include <QMessageBox>
#include <QDesktopServices>

#include "model/NodeModel.h"
#include "nodewidget.h"
#include "ui_nodewidget.h"
#include "mainwindow.h"

NodeWidget::NodeWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::NodeWidget)
{
    ui->setupUi(this);

    connect(ui->btn_add_custom, &QPushButton::clicked, this, &NodeWidget::onCustomAddClicked);

    connect(ui->nodeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button) {
        auto name = button->objectName();
        if (name == "radioButton_websocket") {
            emit nodeSourceChanged(NodeSource::websocket);
        } else if (name == "radioButton_custom") {
            emit nodeSourceChanged(NodeSource::custom);
        }
    });

    m_contextActionRemove = new QAction("Remove", this);
    m_contextActionConnect = new QAction(QIcon(":/assets/images/connect.svg"), "Connect to node", this);
    m_contextActionOpenStatusURL = new QAction(QIcon(":/assets/images/network.png"), "Visit status page", this);
    m_contextActionCopy = new QAction(QIcon(":/assets/images/copy.png"), "Copy", this);
    connect(m_contextActionConnect, &QAction::triggered, this, &NodeWidget::onContextConnect);
    connect(m_contextActionRemove, &QAction::triggered, this, &NodeWidget::onContextCustomNodeRemove);
    connect(m_contextActionOpenStatusURL, &QAction::triggered, this, &NodeWidget::onContextStatusURL);
    connect(m_contextActionCopy, &QAction::triggered, this, &NodeWidget::onContextNodeCopy);
    connect(m_contextActionRemove, &QAction::triggered, this, &NodeWidget::onContextCustomNodeRemove);

    ui->wsView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->customView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->wsView, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowWSContextMenu);
    connect(ui->customView, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowCustomContextMenu);

    connect(ui->customView, &QTreeView::doubleClicked, this, &NodeWidget::onContextConnect);
    connect(ui->wsView, &QTreeView::doubleClicked, this, &NodeWidget::onContextConnect);
}

void NodeWidget::onShowWSContextMenu(const QPoint &pos) {
    m_activeView = ui->wsView;
    FeatherNode node = this->selectedNode();
    if (node.full.isEmpty()) return;

    this->showContextMenu(pos, node);
}

void NodeWidget::onShowCustomContextMenu(const QPoint &pos) {
    m_activeView = ui->customView;
    FeatherNode node = this->selectedNode();
    if (node.full.isEmpty()) return;

    this->showContextMenu(pos, node);
}

void NodeWidget::showContextMenu(const QPoint &pos, const FeatherNode &node) {
    QMenu menu(this);

    if (!node.isActive) {
        menu.addAction(m_contextActionConnect);
    }

    menu.addAction(m_contextActionOpenStatusURL);
    menu.addAction(m_contextActionCopy);

    if (m_activeView == ui->customView)
        menu.addAction(m_contextActionRemove);

    menu.exec(m_activeView->viewport()->mapToGlobal(pos));
}

void NodeWidget::onContextConnect() {
    QObject *obj = sender();
    if (obj == ui->customView)
        m_activeView = ui->customView;
    else
        m_activeView = ui->wsView;

    FeatherNode node = this->selectedNode();
    if (!node.full.isEmpty())
        emit connectToNode(node);
}

void NodeWidget::onContextStatusURL() {
    FeatherNode node = this->selectedNode();
    if (!node.full.isEmpty())
        Utils::externalLinkWarning(this, QString("%1/get_info").arg(node.as_url()));
}

void NodeWidget::onContextNodeCopy() {
    FeatherNode node = this->selectedNode();
    Utils::copyToClipboard(node.full);
}

FeatherNode NodeWidget::selectedNode() {
    QModelIndex index = m_activeView->currentIndex();
    if (!index.isValid()) return FeatherNode();

    FeatherNode node;
    if (m_activeView == ui->customView) {
        node = m_customModel->node(index.row());
    } else {
        node = m_wsModel->node(index.row());
    }
    return node;
}

void NodeWidget::onContextCustomNodeRemove() {
    QModelIndex index = ui->customView->currentIndex();
    if (!index.isValid()) return;
    FeatherNode node = m_customModel->node(index.row());

    auto nodes = m_ctx->nodes->customNodes();
    QMutableListIterator<FeatherNode> i(nodes);
    while (i.hasNext())
        if (i.next() == node)
            i.remove();

    m_ctx->nodes->setCustomNodes(nodes);
}

void NodeWidget::onCustomAddClicked(){
    auto currentNodes = m_ctx->nodes->customNodes();
    auto currentNodesText = QString("");

    for(auto &entry: currentNodes)
        currentNodesText += QString("%1\n").arg(entry.full);

    bool ok;
    QString text = QInputDialog::getMultiLineText(this, "Add custom node(s).", "E.g: user:password@127.0.0.1:18081", currentNodesText, &ok);
    if (!ok || text.isEmpty())
        return;

    QList<FeatherNode> nodesList;
    auto newNodesList = text.split("\n");
    for(auto &newNodeText: newNodesList) {
        newNodeText = newNodeText.replace("\r", "").trimmed();
        if(newNodeText.isEmpty())
            continue;

        auto node = FeatherNode(newNodeText);
        node.custom = true;
        nodesList.append(node);
    }

    m_ctx->nodes->setCustomNodes(nodesList);
}

void NodeWidget::setupUI(AppContext *ctx) {
    m_ctx = ctx;

    auto nodeSource = m_ctx->nodes->source();
    qCritical() << nodeSource;

    if(nodeSource == NodeSource::websocket){
        ui->radioButton_websocket->setChecked(true);
    } else if(nodeSource == NodeSource::custom) {
        ui->radioButton_custom->setChecked(true);
    }

    this->setWSModel(m_ctx->nodes->modelWebsocket);
    this->setCustomModel(m_ctx->nodes->modelCustom);
}

void NodeWidget::setWSModel(NodeModel *model) {
    m_wsModel = model;
    ui->wsView->setModel(m_wsModel);
    ui->wsView->header()->setSectionResizeMode(NodeModel::URL, QHeaderView::Stretch);
    ui->wsView->header()->setSectionResizeMode(NodeModel::Height, QHeaderView::ResizeToContents);
}

void NodeWidget::setCustomModel(NodeModel *model) {
    m_customModel = model;
    ui->customView->setModel(m_customModel);
    ui->customView->header()->setSectionResizeMode(NodeModel::URL, QHeaderView::Stretch);
}

NodeModel* NodeWidget::model() {
    return m_wsModel;
}

NodeWidget::~NodeWidget() {
    delete ui;
}
