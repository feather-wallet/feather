// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "ui_seeddialog.h"
#include "seeddialog.h"

SeedDialog::SeedDialog(const QString &seed, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::SeedDialog)
{
    ui->setupUi(this);
    ui->label_seedIcon->setPixmap(QPixmap(":/assets/images/seed.png").scaledToWidth(64, Qt::SmoothTransformation));
    ui->seed->setPlainText(seed);

    int words = seed.split(" ").size();
    ui->label_warning->setText(QString("<p>Please save these %1 words on paper (order is important). "
                               "This seed will allow you to recover your wallet in case "
                               "of computer failure."
                               "</p>"
                               "<b>WARNING:</b>"
                               "<ul>"
                               "<li>Never disclose your seed.</li>"
                               "<li>Never type it on a website</li>"
                               "<li>Do not store it electronically</li>"
                               "</ul>").arg(words));

    this->adjustSize();
}

SeedDialog::~SeedDialog()
{
    delete ui;
}