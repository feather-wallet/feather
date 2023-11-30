// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_REDDITWIDGET_H
#define FEATHER_REDDITWIDGET_H

#include <QItemSelection>
#include <QMenu>
#include <QWidget>

#include "RedditModel.h"
#include "RedditProxyModel.h"

namespace Ui {
    class RedditWidget;
}

class RedditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RedditWidget(QWidget *parent = nullptr);
    ~RedditWidget();

public slots:
    void linkClicked();

signals:
    void setStatusText(const QString &msg, bool override, int timeout);

private:
    void setupTable();
    void showContextMenu(const QPoint &pos);
    void copyUrl();
    QString getLink(const QString &permaLink);

    QScopedPointer<Ui::RedditWidget> ui;
    RedditModel* const m_model;
    RedditProxyModel* m_proxyModel;
    QMenu *m_contextMenu;
};

#endif // FEATHER_REDDITWIDGET_H
