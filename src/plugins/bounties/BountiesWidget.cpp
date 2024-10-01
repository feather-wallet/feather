// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "BountiesWidget.h"
#include "ui_BountiesWidget.h"

#include <QTableWidget>
#include <QJsonArray>

#include "BountiesModel.h"
#include "utils/Utils.h"
#include "utils/config.h"
#include "utils/WebsocketNotifier.h"

BountiesWidget::BountiesWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::BountiesWidget)
        , m_model(new BountiesModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);

    m_proxyModel = new BountiesProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    ui->tableView->setModel(m_proxyModel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->sortByColumn(3, Qt::DescendingOrder);
    this->setupTable();

    m_contextMenu->addAction("View Bounty", this, &BountiesWidget::linkClicked);
    m_contextMenu->addAction("Donate", this, &BountiesWidget::donateClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &BountiesWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &BountiesWidget::linkClicked);

    connect(websocketNotifier(), &WebsocketNotifier::dataReceived, this, [this](const QString &type, const QJsonValue &json) {
        if (type == "bounties") {
            QJsonArray bounties_data = json.toArray();
            QList<QSharedPointer<BountyEntry>> l;

            for (const auto& entry : bounties_data) {
                QJsonObject obj = entry.toObject();
                auto bounty = new BountyEntry(obj.value("votes").toInt(),
                                              obj.value("title").toString(),
                                              obj.value("amount").toDouble(),
                                              obj.value("link").toString(),
                                              obj.value("address").toString(),
                                              obj.value("status").toString());
                QSharedPointer<BountyEntry> b = QSharedPointer<BountyEntry>(bounty);
                l.append(b);
            }

            m_model->updateBounties(l);
        }
    });

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void BountiesWidget::linkClicked() {
    QModelIndex index = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    auto post = m_model->post(index.row());

    if (post)
        Utils::externalLinkWarning(this, this->getLink(post->link));
}

void BountiesWidget::donateClicked() {
    QModelIndex index = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    auto bounty = m_model->post(index.row());

    if (bounty) {
        emit donate(bounty->donationAddress, QString("Bounty: %1").arg(bounty->title));
    }
}

void BountiesWidget::setupTable() {
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void BountiesWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

QString BountiesWidget::getLink(const QString &permaLink) {
    QString frontend = conf()->get(Config::bountiesFrontend).toString();
    return QString("%1/%2").arg(frontend, permaLink);
}

BountiesWidget::~BountiesWidget() = default;