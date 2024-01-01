// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

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

CalcWindow::~CalcWindow() = default;