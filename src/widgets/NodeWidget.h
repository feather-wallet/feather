// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

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
    void setupUI(QSharedPointer<AppContext> ctx);
    NodeModel* model();

public slots:
    void onCustomAddClicked();
    void onShowWSContextMenu(const QPoint &pos);
    void onShowCustomContextMenu(const QPoint &pos);

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
    QSharedPointer<AppContext> m_ctx;
    NodeModel  *m_customModel;
    NodeModel *m_wsModel;

    QTreeView *m_activeView;

    QAction *m_contextActionConnect;
    QAction *m_contextActionRemove;
    QAction *m_contextActionOpenStatusURL;
    QAction *m_contextActionCopy;
};

#endif // FEATHER_NODEWIDGET_H
