// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_CSSWIDGET_H
#define FEATHER_CSSWIDGET_H

#include <QItemDelegate>
#include <QMenu>
#include <QObject>
#include <QProgressBar>
#include <QWidget>

#include "CCSModel.h"
#include "CCSEntry.h"

namespace Ui {
    class CSSWidget;
}

class CCSWidget : public QWidget
{
Q_OBJECT

public:
    explicit CCSWidget(QWidget *parent = nullptr);
    ~CCSWidget();

signals:
    void fillSendTab(const QString &address, const QString &description);

public slots:
    void donateClicked();

private slots:
    void linkClicked();

private:
    void showContextMenu(const QPoint &pos);

    QScopedPointer<Ui::CSSWidget> ui;
    CCSModel *m_model;
    QMenu *m_contextMenu;
};

#endif // FEATHER_CSSWIDGET_H
