// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_REVUOWIDGET_H
#define FEATHER_REVUOWIDGET_H

#include <QWidget>
#include <QMenu>

#include "RevuoItem.h"

namespace Ui {
    class RevuoWidget;
}

class RevuoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RevuoWidget(QWidget *parent = nullptr);
    ~RevuoWidget();

signals:
    void donate(const QString &address, const QString &description);

public slots:
    void updateItems(const QList<QSharedPointer<RevuoItem>>& items);
    void skinChanged();

private slots:
    void onLinkActivated(const QUrl &link);
    void onSelectItem(int index);
    void onOpenLink();
    void onDonate();

private:
    QScopedPointer<Ui::RevuoWidget> ui;

    QStringList m_items;
    QStringList m_links;
};


#endif //FEATHER_REVUOWIDGET_H
