// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "CCSWidget.h"
#include "ui_CCSWidget.h"

#include <QDesktopServices>
#include <QStandardItemModel>
#include <QTableWidget>

#include "CCSProgressDelegate.h"
#include "utils/Utils.h"

CCSWidget::CCSWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CSSWidget)
        , m_model(new CCSModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(m_model);

    m_contextMenu->addAction("View proposal", this, &CCSWidget::linkClicked);
    m_contextMenu->addAction("Donate", this, &CCSWidget::donateClicked);

    connect(ui->treeView, &QHeaderView::customContextMenuRequested, this, &CCSWidget::showContextMenu);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &CCSWidget::linkClicked);

    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(CCSModel::Title, QHeaderView::Stretch);
}

CCSModel* CCSWidget::model() {
    return m_model;
}

void CCSWidget::linkClicked() {
    QModelIndex index = ui->treeView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry) {
        Utils::externalLinkWarning(this, entry->url);
    }
}

void CCSWidget::donateClicked() {
    QModelIndex index = ui->treeView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry)
        emit selected(*entry);
}

void CCSWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->treeView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->treeView->viewport()->mapToGlobal(pos));
}

CCSWidget::~CCSWidget() = default;