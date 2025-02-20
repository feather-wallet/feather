// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef PLUGIN_H
#define PLUGIN_H

#include <QString>
#include <QWidget>
#include <QTabWidget>

#include "libwalletqt/Wallet.h"

class Plugin : public QObject {
Q_OBJECT

public:
    enum PluginType {
        TAB = 0,
        WIDGET
    };

    virtual QString id() = 0;

    // Used for sorting
    virtual int idx() const = 0;

    // id of parent plugin, plugin is only loaded if parent is available
    virtual QString parent() = 0;
    virtual QString displayName() = 0;
    virtual QString description() = 0;
    virtual QString icon() = 0;

    // register expected websocket data
    virtual QStringList socketData() = 0;
    virtual PluginType type() = 0;
    virtual QWidget* tab() = 0;

    virtual bool configurable() {return false;}
    virtual QDialog* configDialog(QWidget *parent) {return nullptr;}

    // the plugin is automatically enabled if it has any enabled children
    virtual bool implicitEnable() {return false;}
    virtual bool requiresWebsocket() {return true;}

    // insert tab to the left of standard tabs
    virtual bool insertFirst() {return false;}
    virtual void addSubPlugin(Plugin* plugin) {}

    virtual void initialize(Wallet *wallet, QObject *parent) = 0;

    bool hasParent() {return !parent().isEmpty();}

signals:
    void setStatusText(const QString &text, bool override, int timeout);
    void fillSendTab(const QString &address, const QString &description);

public slots:
    virtual void skinChanged() {}
    virtual void uiSetup() {}
    virtual void aboutToQuit() {}

protected:
    Wallet* m_wallet = nullptr;
};

#endif //PLUGIN_H
