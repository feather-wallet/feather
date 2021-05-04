// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "splashdialog.h"
#include "ui_splashdialog.h"

SplashDialog::SplashDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::SplashDialog)
{
    ui->setupUi(this);

    QPixmap pixmap = QPixmap(":/assets/images/key.png");
    ui->icon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    this->adjustSize();
}

void SplashDialog::setMessage(const QString &message) {
    ui->label_message->setText(message);
}

void SplashDialog::setIcon(const QPixmap &icon) {
    ui->icon->setPixmap(icon.scaledToWidth(32, Qt::SmoothTransformation));
}

SplashDialog::~SplashDialog() {
    delete ui;
}
