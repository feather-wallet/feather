// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QDesktopServices>

#include "ccswidget.h"
#include "ui_ccswidget.h"
#include "utils/utils.h"
#include "ccsprogressdelegate.h"

CCSWidget::CCSWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CSSWidget),
        m_model(new CCSModel(this)),
        m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    auto progressDelegate = new CCSProgressDelegate(m_model, this);
    ui->tableView->setModel(m_model);
#ifndef Q_OS_MACOS
    ui->tableView->setItemDelegateForColumn(2, progressDelegate);
#endif
    this->setupTable();

    m_contextMenu->addAction("View proposal", this, &CCSWidget::linkClicked);
    m_contextMenu->addAction("Donate", this, &CCSWidget::donateClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &CCSWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &CCSWidget::linkClicked);
}

CCSModel* CCSWidget::model() {
    return m_model;
}

void CCSWidget::linkClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry)
        Utils::externalLinkWarning(this, entry->url);
}

void CCSWidget::donateClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry)
        emit selected(*entry);
}

void CCSWidget::setupTable() {
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->setColumnWidth(2, 160);
}

void CCSWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

CCSWidget::~CCSWidget() {
    delete ui;
}
