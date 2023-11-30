// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "ExchangeWidget.h"
#include "ui_ExchangeWidget.h"

#include "utils/Icons.h"

ExchangeWidget::ExchangeWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ExchangeWidget)
{
    ui->setupUi(this);
}

void ExchangeWidget::addTab(Plugin *plugin) {
    QWidget* tab = plugin->tab();
    auto icon = icons()->icon(plugin->icon());
    QString name = plugin->displayName();

    ui->tabWidget->addTab(tab, icon, name);
}

ExchangeWidget::~ExchangeWidget() = default;