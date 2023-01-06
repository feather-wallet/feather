// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "RedditWidget.h"
#include "ui_RedditWidget.h"

#include <QDesktopServices>
#include <QStandardItemModel>
#include <QTableWidget>

#include "model/RedditModel.h"
#include "utils/Utils.h"
#include "utils/config.h"

RedditWidget::RedditWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::RedditWidget)
        , m_model(new RedditModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);
    this->setupTable();

    m_contextMenu->addAction("View thread", this, &RedditWidget::linkClicked);
    m_contextMenu->addAction("Copy link", this, &RedditWidget::copyUrl);
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

    if (post)
        Utils::externalLinkWarning(this, this->getLink(post->permalink));
}

void RedditWidget::copyUrl() {
    QModelIndex index = ui->tableView->currentIndex();
    auto post = m_model->post(index.row());

    if (post) {
        Utils::copyToClipboard(this->getLink(post->permalink));
        emit setStatusText("Link copied to clipboard", true, 1000);
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

QString RedditWidget::getLink(const QString &permaLink) {
    QString redditFrontend = config()->get(Config::redditFrontend).toString();
    return QString("https://%1%2").arg(redditFrontend, permaLink);
}

RedditWidget::~RedditWidget() = default;