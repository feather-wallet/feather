// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef REDDITWIDGET_H
#define REDDITWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "model/RedditModel.h"

namespace Ui {
    class RedditWidget;
}

class RedditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RedditWidget(QWidget *parent = nullptr);
    ~RedditWidget();
    RedditModel* model();

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
    QMenu *m_contextMenu;
};

#endif // REDDITWIDGET_H
