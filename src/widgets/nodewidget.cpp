// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QtCore>
#include <QPlainTextEdit>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <utils/config.h>

#include "model/NodeModel.h"
#include "nodewidget.h"
#include "ui_nodewidget.h"
#include "utils/utils.h"
#include "utils/nodes.h"
#include "mainwindow.h"

NodeWidget::NodeWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::NodeWidget),
        m_contextMenu(new QMenu(this)) {
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

    ui->wsView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->customView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->wsView, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowWSContextMenu);
    connect(ui->customView, &QTreeView::customContextMenuRequested, this, &NodeWidget::onShowCustomContextMenu);
}

void NodeWidget::onShowWSContextMenu(const QPoint &pos) {
    QModelIndex index = ui->wsView->indexAt(pos);
    if (!index.isValid()) return;

    FeatherNode node = m_wsModel->node(index.row());
    this->showContextMenu(pos, node);
}

void NodeWidget::onShowCustomContextMenu(const QPoint &pos) {
    QModelIndex index = ui->customView->indexAt(pos);
    if (!index.isValid()) return;

    FeatherNode node = m_customModel->node(index.row());
    this->showContextMenu(pos, node);
}


void NodeWidget::showContextMenu(const QPoint &pos, const FeatherNode &node) {
    bool custom = node.custom;
    m_activeView = custom ? ui->customView : ui->wsView;

    m_contextActionRemove = new QAction("Remove");
    m_contextActionConnect = new QAction("Connect to node");
    m_contextActionConnect->setIcon(QIcon(":/assets/images/connect.svg"));
    m_contextActionOpenStatusURL = new QAction("Visit status page");
    m_contextActionOpenStatusURL->setIcon(QIcon(":/assets/images/network.png"));
    m_contextActionCopy = new QAction("Copy");
    m_contextActionCopy->setIcon(QIcon(":/assets/images/copy.png"));

    if(!node.isActive) {
        connect(m_contextActionConnect, &QAction::triggered, this, &NodeWidget::onContextConnect);
        m_contextMenu->addAction(m_contextActionConnect);
    }

    m_contextMenu->addAction(m_contextActionOpenStatusURL);
    m_contextMenu->addAction(m_contextActionCopy);

    if(custom) {
        connect(m_contextActionRemove, &QAction::triggered, this, &NodeWidget::onContextCustomNodeRemove);
        m_contextMenu->addAction(m_contextActionRemove);

        connect(m_contextActionOpenStatusURL, &QAction::triggered, this, &NodeWidget::onContextCustomStatusURL);
        connect(m_contextActionCopy, &QAction::triggered, this, &NodeWidget::onContextCustomNodeCopy);
    } else {
        connect(m_contextActionOpenStatusURL, &QAction::triggered, this, &NodeWidget::onContextWSStatusURL);
        connect(m_contextActionCopy, &QAction::triggered, this, &NodeWidget::onContextWSNodeCopy);
    }

    m_contextMenu->exec(m_activeView->viewport()->mapToGlobal(pos));
    m_contextActionRemove->deleteLater();
    m_contextActionConnect->deleteLater();
    m_contextActionOpenStatusURL->deleteLater();
    m_contextActionCopy->deleteLater();
}

void NodeWidget::onContextConnect() {
    QModelIndex index = m_activeView->currentIndex();
    if (!index.isValid()) return;

    if(m_activeView->objectName() == "wsView"){
        FeatherNode node = m_wsModel->node(index.row());
        emit connectToNode(node);
    } else {
        FeatherNode node = m_customModel->node(index.row());
        emit connectToNode(node);
    }
}

void NodeWidget::onContextWSStatusURL() {
    QModelIndex index = ui->wsView->currentIndex();
    if (!index.isValid()) return;
    FeatherNode node = m_wsModel->node(index.row());
    Utils::externalLinkWarning(node.as_url());
}

void NodeWidget::onContextCustomStatusURL() {
    QModelIndex index = ui->customView->currentIndex();
    if (!index.isValid()) return;
    FeatherNode node = m_customModel->node(index.row());
    Utils::externalLinkWarning(node.as_url());
}

void NodeWidget::onContextDisconnect() {
    QModelIndex index = ui->customView->currentIndex();
    if (!index.isValid()) return;
    FeatherNode node = m_customModel->node(index.row());

    Utils::copyToClipboard(node.full);
}

void NodeWidget::onContextWSNodeCopy() {
    QModelIndex index = ui->wsView->currentIndex();
    if (!index.isValid()) return;
    FeatherNode node = m_wsModel->node(index.row());

    Utils::copyToClipboard(node.full);
}

void NodeWidget::onContextCustomNodeCopy() {

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
