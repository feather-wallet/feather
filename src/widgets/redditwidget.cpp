// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QDesktopServices>

#include "model/RedditModel.h"
#include "redditwidget.h"
#include "ui_redditwidget.h"
#include "utils/utils.h"
#include "utils/config.h"

RedditWidget::RedditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RedditWidget),
    m_model(new RedditModel(this)),
    m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);
    this->setupTable();

    m_contextMenu->addAction("View thread", this, &RedditWidget::linkClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &RedditWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &RedditWidget::linkClicked);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

RedditModel* RedditWidget::model() {
    return m_model;
}

void RedditWidget::linkClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto post = m_model->post(index.row());

    if (post != nullptr) {
        QString redditFrontend = config()->get(Config::redditFrontend).toString();
        QString link = QString("https://%1%2").arg(redditFrontend, post->permalink);

        Utils::externalLinkWarning(this, link);
    }
}

void RedditWidget::setupTable() {
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch);
}

void RedditWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

RedditWidget::~RedditWidget() {
    delete ui;
}
