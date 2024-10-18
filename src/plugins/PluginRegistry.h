// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include "Plugin.h"
#include "utils/config.h"
#include "constants.h"

class PluginRegistry {
public:
    static void registerPlugin(Plugin* plugin) {
        getInstance().plugins.append(plugin);
        getInstance().pluginMap[plugin->id()] = plugin;

        std::sort(getInstance().plugins.begin(), getInstance().plugins.end(), [](Plugin *a, Plugin *b) {
            return a->idx() < b->idx();
        });
    }

    void registerPluginCreator(const std::function<Plugin*()>& creator) {
        plugin_creators.append(creator);
    }

    static const QList<Plugin*>& getPlugins() {
        return getInstance().plugins;
    }

    static Plugin* getPlugin(const QString& id) {
        return getInstance().pluginMap.value(id, nullptr);
    }

    static const QList<std::function<Plugin*()>>& getPluginCreators() {
        return getInstance().plugin_creators;
    }

    bool isPluginEnabled(const QString &id) {
        if (!pluginMap.contains(id) || (QString::compare(id,"atomic")==0 && constants::networkType==NetworkType::TESTNET)) {
            return false;
        }

        Plugin* plugin = pluginMap[id];

        // Don't load plugins that require the websocket connection if it is disabled
        bool websocketDisabled = conf()->get(Config::disableWebsocket).toBool();
        if (websocketDisabled && plugin->requiresWebsocket()) {
            return false;
        }

        QStringList enabledPlugins = conf()->get(Config::enabledPlugins).toStringList();
        if (enabledPlugins.contains(id) && !plugin->implicitEnable()) {
            return true;
        }

        bool enabled = false;
        if (plugin->implicitEnable()) {
            for (const auto& child : plugins) {
                if (child->parent() == plugin->id() && enabledPlugins.contains(child->id())) {
                    enabled = true;
                }
            }
        }

        return enabled;
    }

private:
    QMap<QString, Plugin*> pluginMap;
    QList<Plugin*> plugins;
    QList<std::function<Plugin*()>> plugin_creators;

public:
    static PluginRegistry& getInstance() {
        static PluginRegistry instance;
        return instance;
    }
};

#endif //PLUGINREGISTRY_H
