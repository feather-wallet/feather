// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PLUGINWIDGET_H
#define PLUGINWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "plugins/Plugin.h"

namespace Ui {
    class PluginWidget;
}

class PluginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginWidget(QWidget *parent = nullptr);
    ~PluginWidget();

private slots:
    void pluginSelected(QTreeWidgetItem* item, int column);
    void pluginToggled(QTreeWidgetItem* item, int column);
    void deselectAll();
    void selectAll();
    void selectTreeItems(QTreeWidgetItem *item, bool select);
    void configurePlugin();

signals:
    void pluginConfigured(const QString &id);

private:
    void setupItem(QTreeWidgetItem *item, Plugin *plugin);

    QScopedPointer<Ui::PluginWidget> ui;
    QMap<QString, QTreeWidgetItem*> m_topLevelPlugins;
    QList<QString> m_checkable;
    Plugin* m_selectedPlugin = nullptr;
};


#endif //PLUGINWIDGET_H
