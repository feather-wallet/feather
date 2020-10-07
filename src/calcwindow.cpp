// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "calcwindow.h"
#include "mainwindow.h"

#include "ui_calcwindow.h"

CalcWindow::CalcWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::CalcWindow)
{
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags|Qt::WindowStaysOnTopHint); // on top

    ui->setupUi(this);
    this->setWindowIcon(QIcon("://assets/images/coldcard.png"));

    connect(AppContext::prices, &Prices::fiatPricesUpdated, this, &CalcWindow::initFiat);
    connect(AppContext::prices, &Prices::cryptoPricesUpdated, this, &CalcWindow::initCrypto);
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
