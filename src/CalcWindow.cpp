// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CalcWindow.h"
#include "ui_CalcWindow.h"

#include "utils/AppData.h"
#include "utils/Icons.h"

CalcWindow::CalcWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CalcWindow)
{
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags|Qt::WindowStaysOnTopHint); // on top

    ui->setupUi(this);
    this->setWindowIcon(icons()->icon("gnome-calc.png"));
}

void CalcWindow::closeEvent(QCloseEvent *foo) {
    emit closed();
}

CalcWindow::~CalcWindow() = default;