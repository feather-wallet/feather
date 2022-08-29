// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "config-feather.h"
#include "utils/Utils.h"

AboutDialog::AboutDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // cute fox (c) Diego "rehrar" Salazar :-D
    QPixmap p(":assets/images/cutexmrfox.png");
    ui->aboutImage->setPixmap(p.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto about = Utils::fileOpenQRC(":assets/about.txt");
    auto about_text = Utils::barrayToString(about);
    about_text = about_text.replace("<feather_version>", FEATHER_VERSION);
    about_text = about_text.replace("<feather_git_head>", FEATHER_COMMIT);
    about_text = about_text.replace("<current_year>", QString::number(QDate::currentDate().year()));
    ui->copyrightText->setPlainText(about_text);

    auto ack = Utils::fileOpenQRC(":assets/ack.txt");
    auto ack_text = Utils::barrayToString(ack);
    ui->ackText->setText(ack_text);

    this->adjustSize();
}

AboutDialog::~AboutDialog() = default;