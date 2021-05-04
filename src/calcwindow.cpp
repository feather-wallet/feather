// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "calcwindow.h"
#include "mainwindow.h"
#include "utils/Icons.h"
#include "utils/AppData.h"

#include "ui_calcwindow.h"

CalcWindow::CalcWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::CalcWindow)
{
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags|Qt::WindowStaysOnTopHint); // on top

    ui->setupUi(this);
    this->setWindowIcon(icons()->icon("gnome-calc.png"));

    connect(&appData()->prices, &Prices::fiatPricesUpdated, this, &CalcWindow::initFiat);
    connect(&appData()->prices, &Prices::cryptoPricesUpdated, this, &CalcWindow::initCrypto);
}

void CalcWindow::initFiat() {
    this->ui->calcWidget->initFiat();
}

void CalcWindow::initCrypto() {
    this->ui->calcWidget->initCrypto();
}

void CalcWindow::closeEvent(QCloseEvent *foo) {
    emit closed();
}

CalcWindow::~CalcWindow() {
    delete ui;
}
