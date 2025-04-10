// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QMessageBox>
#include <QDate>
#include <QSslSocket>

#include "utils/Utils.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QPixmap p(":assets/images/appicons/256x256.png");
    ui->aboutImage->setPixmap(p.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto about = Utils::fileOpenQRC(":assets/about.txt");
    auto about_text = Utils::barrayToString(about);
    about_text = about_text.replace("<feather_version>", Utils::getVersion());
    ui->copyrightText->setPlainText(about_text);

    auto ack = Utils::fileOpenQRC(":assets/ack.txt");
    auto ack_text = Utils::barrayToString(ack);
    ui->ackText->setText(ack_text);

    ui->label_featherVersion->setText(FEATHER_VERSION);
    ui->label_moneroVersion->setText(MONERO_VERSION);
    ui->label_qtVersion->setText(QT_VERSION_STR);
    ui->label_torVersion->setText(TOR_VERSION);
    ui->label_sslVersion->setText(QSslSocket::sslLibraryVersionString());

    this->adjustSize();
}

AboutDialog::~AboutDialog() = default;