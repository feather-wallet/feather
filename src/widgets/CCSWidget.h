// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CSSWIDGET_H
#define FEATHER_CSSWIDGET_H

#include <QItemDelegate>
#include <QMenu>
#include <QObject>
#include <QProgressBar>
#include <QWidget>

#include "appcontext.h"
#include "model/CCSModel.h"
#include "widgets/CCSEntry.h"

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

    QScopedPointer<Ui::CSSWidget> ui;
    CCSModel *m_model;
    QMenu *m_contextMenu;
};

#endif // FEATHER_CSSWIDGET_H
