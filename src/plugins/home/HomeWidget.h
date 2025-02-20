// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>

#include "plugins/Plugin.h"

namespace Ui {
    class HomeWidget;
}

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidget(QWidget *parent = nullptr);
    ~HomeWidget();

    void addPlugin(Plugin *plugin);
    void aboutToQuit();
    void uiSetup();

private:
    QScopedPointer<Ui::HomeWidget> ui;
};

#endif //HOMEWIDGET_H
