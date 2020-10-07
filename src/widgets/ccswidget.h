// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef CSSWIDGET_H
#define CSSWIDGET_H

#include <QMenu>
#include <QObject>
#include <QWidget>
#include <QProgressBar>
#include <QItemDelegate>

#include "widgets/CCSEntry.h"
#include "model/CCSModel.h"
#include "appcontext.h"

namespace Ui {
    class CSSWidget;
}

class CCSWidget : public QWidget
{
Q_OBJECT

public:
    explicit CCSWidget(QWidget *parent = nullptr);
    ~CCSWidget();
    CCSModel *model();

signals:
    void selected(CCSEntry entry);

public slots:
    void donateClicked();

private slots:
    void linkClicked();

private:
    void setupTable();
    void showContextMenu(const QPoint &pos);

    Ui::CSSWidget *ui;
    CCSModel *m_model;
    QMenu *m_contextMenu;
};

#endif // CSSWIDGET_H
