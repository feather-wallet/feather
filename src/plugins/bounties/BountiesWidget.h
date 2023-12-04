// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_BOUNTIESWIDGET_H
#define FEATHER_BOUNTIESWIDGET_H

#include <QItemSelection>
#include <QMenu>
#include <QWidget>

#include "BountiesModel.h"
#include "BountiesProxyModel.h"

namespace Ui {
    class BountiesWidget;
}

class BountiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BountiesWidget(QWidget *parent = nullptr);
    ~BountiesWidget() override;

public slots:
    void linkClicked();

signals:
    void setStatusText(const QString &msg, bool override, int timeout);
    void donate(const QString &address, const QString &description);

private:
    void setupTable();
    void showContextMenu(const QPoint &pos);
    void donateClicked();
    QString getLink(const QString &permaLink);

    QScopedPointer<Ui::BountiesWidget> ui;
    BountiesModel *m_model;
    BountiesProxyModel *m_proxyModel;
    QMenu *m_contextMenu;
};

#endif //FEATHER_BOUNTIESWIDGET_H
