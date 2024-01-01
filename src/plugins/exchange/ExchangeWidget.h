// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef EXCHANGEWIDGET_H
#define EXCHANGEWIDGET_H

#include <QWidget>
#include <QTabWidget>

#include "plugins/Plugin.h"

namespace Ui {
    class ExchangeWidget;
}

class ExchangeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExchangeWidget(QWidget *parent = nullptr);
    ~ExchangeWidget();

    void addTab(Plugin *plugin);

private:
    QScopedPointer<Ui::ExchangeWidget> ui;
};


#endif //EXCHANGEWIDGET_H
