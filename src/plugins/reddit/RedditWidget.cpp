// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "RedditWidget.h"
#include "ui_RedditWidget.h"

#include <QTableWidget>

#include "RedditModel.h"
#include "utils/Utils.h"
#include "utils/config.h"
#include "utils/WebsocketNotifier.h"

RedditWidget::RedditWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::RedditWidget)
        , m_model(new RedditModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);

    m_proxyModel = new RedditProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    ui->tableView->setModel(m_proxyModel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->sortByColumn(2, Qt::DescendingOrder);
    this->setupTable();

    m_contextMenu->addAction("View thread", this, &RedditWidget::linkClicked);
    m_contextMenu->addAction("Copy link", this, &RedditWidget::copyUrl);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &RedditWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &RedditWidget::linkClicked);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(websocketNotifier(), &WebsocketNotifier::dataReceived, this, [this](const QString& type, const QJsonValue& json) {
        if (type == "reddit") {
            QJsonArray reddit_data = json.toArray();
            QList<QSharedPointer<RedditPost>> l;

            for (auto &&entry: reddit_data) {
                auto obj = entry.toObject();
                auto redditPost = new RedditPost(
                        obj.value("title").toString(),
                        obj.value("author").toString(),
                        obj.value("permalink").toString(),
                        obj.value("comments").toInt());
                QSharedPointer<RedditPost> r = QSharedPointer<RedditPost>(redditPost);
                l.append(r);
            }

            m_model->updatePosts(l);
        }
    });
}

void RedditWidget::linkClicked() {
    QModelIndex index = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    auto post = m_model->post(index.row());

    if (post)
        Utils::externalLinkWarning(this, this->getLink(post->permalink));
}

void RedditWidget::copyUrl() {
    QModelIndex index = m_proxyModel->mapToSource(ui->tableView->currentIndex());
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
    QString redditFrontend = conf()->get(Config::redditFrontend).toString();
    return QString("https://%1%2").arg(redditFrontend, permaLink);
}

RedditWidget::~RedditWidget() = default;