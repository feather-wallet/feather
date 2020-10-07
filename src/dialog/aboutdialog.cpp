// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "utils/utils.h"
#include "config-feather.h"

AboutDialog::AboutDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));
    // cute fox (c) Diego "rehrar" Salazar :-D
    QPixmap p(":assets/images/cutexmrfox.png");
    ui->aboutImage->setPixmap(p.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto about = Utils::fileOpenQRC(":assets/about.txt");
    auto about_text = Utils::barrayToString(about);
    about_text = about_text.replace("<feather_version>", FEATHER_VERSION);
    about_text = about_text.replace("<feather_git_head>", FEATHER_BRANCH);
    about_text = about_text.replace("<current_year>", QString::number(QDate::currentDate().year()));
    ui->copyrightText->setPlainText(about_text);

    auto ack = Utils::fileOpenQRC(":assets/ack.txt");
    auto ack_text = Utils::barrayToString(ack);
    ui->ackText->setText(ack_text);

    auto sm = QApplication::font();
    auto font = QApplication::font();
    sm.setPointSize(sm.pointSize() - 2);
    this->m_model = new QStandardItemModel(this);
    this->m_model->setHorizontalHeaderItem(0, Utils::qStandardItem("Name", sm));
    this->m_model->setHorizontalHeaderItem(1, Utils::qStandardItem("Email", sm));
    ui->authorView->setModel(this->m_model);

    int i = 0;
    auto contributors = Utils::barrayToString(Utils::fileOpenQRC(":assets/contributors.txt"));
    for(const auto &line: contributors.split("\n")){
        // too lazy for regex #sorry #notsorry
        auto name = line.left(line.indexOf("<")).trimmed();
        auto nameItem = Utils::qStandardItem(name, font);
        auto email = line.mid(line.indexOf("<")+1, line.length()).replace(">", "").trimmed();
        auto emailItem = Utils::qStandardItem(email, font);

        this->m_model->setItem(i, 0, nameItem);
        this->m_model->setItem(i, 1, emailItem);
        i++;
    }

    ui->authorView->header()->setSectionResizeMode(QHeaderView::Stretch);

    this->adjustSize();
}

AboutDialog::~AboutDialog() {
    delete ui;
}

