// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicWindow.h"
#include "ui_AtomicWindow.h"

#include "utils/AppData.h"
#include "utils/Icons.h"

AtomicWindow::AtomicWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::AtomicWindow)
{
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags|Qt::WindowStaysOnTopHint); // on top

    ui->setupUi(this);
    this->setWindowIcon(icons()->icon("gnome-Atomic.png"));
}

AtomicWindow::~AtomicWindow() = default;