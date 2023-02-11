// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_NODEWIDGET_H
#define FEATHER_NODEWIDGET_H

#include <QItemSelection>
#include <QTreeView>
#include <QWidget>

#include "appcontext.h"
#include "model/NodeModel.h"
#include "utils/nodes.h"

namespace Ui {
    class NodeWidget;
}

class NodeModel;
class NodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NodeWidget(QWidget *parent = nullptr);
    ~NodeWidget();
    void setWSModel(NodeModel *model);
    void setCustomModel(NodeModel *model);
    void setupUI(Nodes *nodes);
    void setCanConnect(bool canConnect);
    NodeModel* model();

public slots:
    void onCustomAddClicked();
    void onShowWSContextMenu(const QPoint &pos);
    void onShowCustomContextMenu(const QPoint &pos);
    void onWebsocketStatusChanged();

private slots:
    void onContextConnect();
    void onContextCustomNodeRemove();
    void onContextStatusURL();
    void onContextNodeCopy();

signals:
    void connectToNode(FeatherNode node);
    void nodeSourceChanged(NodeSource nodeSource);

private:
    void showContextMenu(const QPoint &pos, const FeatherNode &node);
    FeatherNode selectedNode();

    QScopedPointer<Ui::NodeWidget> ui;
    Nodes *m_nodes;
    NodeModel  *m_customModel;
    NodeModel *m_wsModel;
    bool m_canConnect = true;

    QTreeView *m_activeView;

    QAction *m_contextActionConnect;
    QAction *m_contextActionRemove;
    QAction *m_contextActionOpenStatusURL;
    QAction *m_contextActionCopy;
};

#endif // FEATHER_NODEWIDGET_H
