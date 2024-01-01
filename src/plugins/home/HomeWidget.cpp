// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "HomeWidget.h"
#include "ui_HomeWidget.h"

#include "utils/config.h"

HomeWidget::HomeWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::HomeWidget)
{
    ui->setupUi(this);
}

void HomeWidget::addPlugin(Plugin *plugin)
{
    if (plugin->type() == Plugin::TAB) {
        ui->tabHomeWidget->addTab(plugin->tab(), plugin->displayName());
    }
    else if (plugin->type() == Plugin::WIDGET) {
        ui->widgetLayout->addWidget(plugin->tab());
    }
}

void HomeWidget::uiSetup() {
    ui->tabHomeWidget->setCurrentIndex(conf()->get(Config::homeWidget).toInt());
}

void HomeWidget::aboutToQuit() {
    conf()->set(Config::homeWidget, ui->tabHomeWidget->currentIndex());
}

HomeWidget::~HomeWidget() = default;