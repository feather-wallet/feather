// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QItemSelection>
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
    void setupUI(AppContext *ctx);
    NodeModel* model();

public slots:
    void onCustomAddClicked();
    void onShowWSContextMenu(const QPoint &pos);
    void onShowCustomContextMenu(const QPoint &pos);

private slots:
    void onContextConnect();
    void onContextDisconnect();
    void onContextCustomNodeRemove();
    void onContextWSStatusURL();
    void onContextCustomStatusURL();
    void onContextWSNodeCopy();
    void onContextCustomNodeCopy();

signals:
    void connectToNode(FeatherNode node);
    void nodeSourceChanged(NodeSource nodeSource);

private:
    AppContext *m_ctx;
    Ui::NodeWidget *ui;
    NodeModel* m_customModel;
    NodeModel* m_wsModel;

    QTreeView *m_activeView;

    QMenu *m_contextMenu;
    QAction *m_contextActionConnect;
    QAction *m_contextActionDisconnect;
    QAction *m_contextActionRemove;
    QAction *m_contextActionOpenStatusURL;
    QAction *m_contextActionCopy;

    void showContextMenu(const QPoint &pos, const FeatherNode &node);
};

#endif // NODEWIDGET_H
