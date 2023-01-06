// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SplashDialog.h"
#include "ui_SplashDialog.h"

#include "utils/Icons.h"

SplashDialog::SplashDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::SplashDialog)
{
    ui->setupUi(this);

    this->setWindowIcon(icons()->icon("appicon/64x64"));

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

SplashDialog::~SplashDialog() = default;