// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_HISTORYVIEW_H
#define FEATHER_HISTORYVIEW_H

#include <QKeyEvent>
#include <QTreeView>
#include <QActionGroup>

#include "TransactionHistoryModel.h"
#include "TransactionHistoryProxyModel.h"

class HistoryView : public QTreeView
{
    Q_OBJECT

public:
    explicit HistoryView(QWidget* parent = nullptr);
    void setHistoryModel(TransactionHistoryProxyModel *model);

    void setSearchMode(bool mode);
    QByteArray viewState() const;
    bool setViewState(const QByteArray& state);
    QModelIndex getCurrentIndex();
    TransactionHistoryModel* sourceModel();

    QMenu* getMenu();

private slots:
    void showHeaderMenu(const QPoint& position);
    void toggleColumnVisibility(QAction* action);
    void showFullTxid(bool enabled);
    void fitColumnsToWindow();
    void fitColumnsToContents();
    void resetViewToDefaults();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    TransactionHistoryProxyModel* m_model;
    bool m_inSearchMode = false;
    bool m_columnsNeedRelayout = true;
    bool m_showTxidColumn = false;

    QMenu* m_headerMenu;
    QActionGroup* m_columnActions;
};


#endif //FEATHER_HISTORYVIEW_H
