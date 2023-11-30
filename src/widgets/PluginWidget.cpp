// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PluginWidget.h"
#include "ui_PluginWidget.h"

#include <QTreeWidget>
#include "utils/config.h"
#include "utils/Icons.h"
#include "plugins/PluginRegistry.h"

PluginWidget::PluginWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::PluginWidget)
{
    ui->setupUi(this);

    ui->frameRestart->setVisible(conf()->get(Config::restartRequired).toBool());
    ui->frameRestart->setInfo(icons()->icon("settings_disabled_32px.png"), "Restart required to enable/disable plugins.");

    const QStringList enabledPlugins = conf()->get(Config::enabledPlugins).toStringList();

    for (const auto plugin : PluginRegistry::getPlugins()) {
        if (!plugin->parent().isEmpty()) {
            continue;
        }

        auto* item = new QTreeWidgetItem(ui->plugins);

        this->setupItem(item, plugin);
        if (enabledPlugins.contains(plugin->id()) && !plugin->implicitEnable()) {
            item->setCheckState(0, Qt::Checked);
        }

        m_topLevelPlugins[plugin->id()] = item;
    }

    for (const auto plugin : PluginRegistry::getPlugins()) {
        QTreeWidgetItem *item;

        if (plugin->parent().isEmpty()) {
            continue;
        }

        if (m_topLevelPlugins.contains(plugin->parent())) {
            item = new QTreeWidgetItem(m_topLevelPlugins[plugin->parent()]);
        } else {
            qWarning() << "Top level plugin not found: " << plugin->id();
            continue;
        }

        this->setupItem(item, plugin);
        if (enabledPlugins.contains(plugin->id())) {
            item->setCheckState(0, Qt::Checked);
        }
    }

    ui->plugins->expandAll();
    ui->plugins->setHeaderHidden(true);

    connect(ui->plugins, &QTreeWidget::itemClicked, this, &PluginWidget::pluginSelected);
    connect(ui->plugins, &QTreeWidget::itemClicked, this, &PluginWidget::pluginToggled);

    connect(ui->btn_deselectAll, &QPushButton::clicked, this, &PluginWidget::deselectAll);
    connect(ui->btn_selectAll, &QPushButton::clicked, this, &PluginWidget::selectAll);
    connect(ui->btn_configure, &QPushButton::clicked, this, &PluginWidget::configurePlugin);
}

void PluginWidget::pluginSelected(QTreeWidgetItem* item, int column) {
    QString pluginID = item->data(0, Qt::UserRole).toString();
    Plugin* plugin = PluginRegistry::getPlugin(pluginID);

    bool enable = false;
    if (plugin && plugin->configurable()) {
        enable = true;
    }

    ui->btn_configure->setEnabled(enable);
    m_selectedPlugin = plugin;
}

void PluginWidget::pluginToggled(QTreeWidgetItem* item, int column) {
    QStringList enabledPlugins = conf()->get(Config::enabledPlugins).toStringList();

    QString pluginID = item->data(0, Qt::UserRole).toString();
    bool checked = (item->checkState(0) == Qt::Checked);
    bool toggled = checked ^ enabledPlugins.contains(pluginID);
    if (!toggled) {
        return;
    }

    if (checked) {
        enabledPlugins.append(pluginID);
    } else {
        enabledPlugins.removeAll(pluginID);
    }

    conf()->set(Config::enabledPlugins, enabledPlugins);
    qDebug() << "Enabled plugins: " << enabledPlugins;

    if (toggled) {
        conf()->set(Config::restartRequired, true);
        ui->frameRestart->show();
    }
}

void PluginWidget::setupItem(QTreeWidgetItem *item, Plugin *plugin) {
    item->setIcon(0, icons()->icon(plugin->icon()));
    item->setText(0, plugin->displayName());
    item->setData(0, Qt::UserRole, plugin->id());

    if (!plugin->implicitEnable()) {
        m_checkable.append(plugin->id());
        item->setCheckState(0, Qt::Unchecked);
    }

    // if (plugin->requiresWebsocket() && conf()->get(Config::disableWebsocket).toBool()) {
    //     item->setDisabled(true);
    //     item->setToolTip(0, "This plugin requires the websocket connection to be enabled. Go to Network -> Websocket.");
    //
    //     if (!plugin->implicitEnable()) {
    //         item->setCheckState(0, Qt::Unchecked);
    //     }
    // }
}

void PluginWidget::deselectAll() {
    this->selectTreeItems(ui->plugins->invisibleRootItem(), false);
}

void PluginWidget::selectAll() {
    this->selectTreeItems(ui->plugins->invisibleRootItem(), true);
}

void PluginWidget::configurePlugin() {
    if (!m_selectedPlugin) {
        return;
    }
    m_selectedPlugin->configDialog(this)->exec();
    emit pluginConfigured(m_selectedPlugin->id());
}

void PluginWidget::selectTreeItems(QTreeWidgetItem *item, bool select) {
    if (!item) return;

    // Truly demented Qt API
    if (m_checkable.contains(item->data(0, Qt::UserRole).toString())) {
        item->setCheckState(0, select ? Qt::Checked : Qt::Unchecked);
        this->pluginToggled(item, 0);
    }

    for (int i = 0; i < item->childCount(); ++i)
    {
        selectTreeItems(item->child(i), select);
    }
}

PluginWidget::~PluginWidget() = default;