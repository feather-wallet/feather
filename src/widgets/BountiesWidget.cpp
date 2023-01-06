// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "BountiesWidget.h"
#include "ui_BountiesWidget.h"

#include <QDesktopServices>
#include <QStandardItemModel>
#include <QTableWidget>

#include "model/BountiesModel.h"
#include "utils/Utils.h"
#include "utils/config.h"

BountiesWidget::BountiesWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::BountiesWidget)
        , m_model(new BountiesModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);
    this->setupTable();

    m_contextMenu->addAction("View Bounty", this, &BountiesWidget::linkClicked);
    m_contextMenu->addAction("Donate", this, &BountiesWidget::donateClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &BountiesWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &BountiesWidget::linkClicked);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

BountiesModel * BountiesWidget::model() {
    return m_model;
}

void BountiesWidget::linkClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto post = m_model->post(index.row());

    if (post)
        Utils::externalLinkWarning(this, this->getLink(post->link));
}

void BountiesWidget::donateClicked() {
    QModelIndex index = ui->tableView->currentIndex();
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
    QString frontend = config()->get(Config::bountiesFrontend).toString();
    return QString("%1/%2").arg(frontend, permaLink);
}

BountiesWidget::~BountiesWidget() = default;