// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "coinswidget.h"
#include "ui_coinswidget.h"
#include "model/ModelUtils.h"
#include "utils/utils.h"
#include "dialog/outputinfodialog.h"
#include "dialog/outputsweepdialog.h"
#include "mainwindow.h"

#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>

CoinsWidget::CoinsWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CoinsWidget)
        , m_headerMenu(new QMenu(this))
        , m_copyMenu(new QMenu("Copy",this))
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    // header context menu
    ui->coins->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showSpentAction = m_headerMenu->addAction("Show spent outputs", this, &CoinsWidget::setShowSpent);
    m_showSpentAction->setCheckable(true);
    connect(ui->coins->header(), &QHeaderView::customContextMenuRequested, this, &CoinsWidget::showHeaderMenu);

    // copy menu
    m_copyMenu->setIcon(QIcon(":/assets/images/copy.png"));
    m_copyMenu->addAction("Public key", this, [this]{copy(copyField::PubKey);});
    m_copyMenu->addAction("Key Image", this, [this]{copy(copyField::KeyImage);});
    m_copyMenu->addAction("Transaction ID", this, [this]{copy(copyField::TxID);});
    m_copyMenu->addAction("Address", this, [this]{copy(copyField::Address);});
    m_copyMenu->addAction("Height", this, [this]{copy(copyField::Height);});
    m_copyMenu->addAction("Amount", this, [this]{copy(copyField::Amount);});

    // context menu
    ui->coins->setContextMenuPolicy(Qt::CustomContextMenu);

    m_thawOutputAction = new QAction("Thaw output", this);
    m_freezeOutputAction = new QAction("Freeze output", this);

    m_freezeAllSelectedAction = new QAction("Freeze selected", this);
    m_thawAllSelectedAction = new QAction("Thaw selected", this);

    m_viewOutputAction = new QAction(QIcon(":/assets/images/info.png"), "Details", this);
    m_sweepOutputAction = new QAction("Sweep output", this);
    connect(m_freezeOutputAction, &QAction::triggered, this, &CoinsWidget::freezeOutput);
    connect(m_thawOutputAction, &QAction::triggered, this, &CoinsWidget::thawOutput);
    connect(m_viewOutputAction, &QAction::triggered, this, &CoinsWidget::viewOutput);
    connect(m_sweepOutputAction, &QAction::triggered, this, &CoinsWidget::onSweepOutput);

    connect(m_freezeAllSelectedAction, &QAction::triggered, this, &CoinsWidget::freezeAllSelected);
    connect(m_thawAllSelectedAction, &QAction::triggered, this, &CoinsWidget::thawAllSelected);

    connect(ui->coins, &QTreeView::customContextMenuRequested, this, &CoinsWidget::showContextMenu);
    connect(ui->coins, &QTreeView::doubleClicked, this, &CoinsWidget::viewOutput);
}

void CoinsWidget::setModel(CoinsModel * model, Coins * coins) {
    m_coins = coins;
    m_model = model;
    m_proxyModel = new CoinsProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui->coins->setModel(m_proxyModel);
    ui->coins->setColumnHidden(CoinsModel::Spent, true);
    ui->coins->setColumnHidden(CoinsModel::SpentHeight, true);
    ui->coins->setColumnHidden(CoinsModel::Frozen, true);

    if (!m_ctx->currentWallet->viewOnly()) {
        ui->coins->setColumnHidden(CoinsModel::KeyImageKnown, true);
    } else {
        ui->coins->setColumnHidden(CoinsModel::KeyImageKnown, false);
    }

    ui->coins->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->coins->header()->setSectionResizeMode(CoinsModel::AddressLabel, QHeaderView::Stretch);
    ui->coins->header()->setSortIndicator(CoinsModel::BlockHeight, Qt::DescendingOrder);
    ui->coins->setSortingEnabled(true);
}

void CoinsWidget::showContextMenu(const QPoint &point) {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    auto *menu = new QMenu(ui->coins);
    if (list.size() > 1) {
        menu->addAction(m_freezeAllSelectedAction);
        menu->addAction(m_thawAllSelectedAction);
    }
    else {
        QModelIndex index = ui->coins->indexAt(point);
        if (!index.isValid()) {
            return;
        }

        int row = m_proxyModel->mapToSource(index).row();

        bool isSpent, isFrozen, isUnlocked;
        m_coins->coin(row, [&isSpent, &isFrozen, &isUnlocked](CoinsInfo &c) {
            isSpent = c.spent();
            isFrozen = c.frozen();
            isUnlocked = c.unlocked();
        });

        menu->addMenu(m_copyMenu);

        if (!isSpent) {
            isFrozen ? menu->addAction(m_thawOutputAction) : menu->addAction(m_freezeOutputAction);
        }
        if (!isFrozen && isUnlocked) {
            menu->addAction(m_sweepOutputAction);
        }
        menu->addAction(m_viewOutputAction);
    }

    menu->popup(ui->coins->viewport()->mapToGlobal(point));
}

void CoinsWidget::showHeaderMenu(const QPoint& position)
{
    m_headerMenu->popup(QCursor::pos());
}

void CoinsWidget::setShowSpent(bool show)
{
    if(!m_proxyModel) return;
    m_proxyModel->setShowSpent(show);
}

void CoinsWidget::freezeOutput() {
    QModelIndex index = ui->coins->currentIndex();
    emit freeze(m_proxyModel->mapToSource(index).row());
}

void CoinsWidget::freezeAllSelected() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    QVector<int> indexes;
    for (QModelIndex index: list) {
        indexes.push_back(m_proxyModel->mapToSource(index).row()); // todo: will segfault if index get invalidated
    }
    emit freezeMulti(indexes);
}

void CoinsWidget::thawOutput() {
    QModelIndex index = ui->coins->currentIndex();
    emit thaw(m_proxyModel->mapToSource(index).row());
}

void CoinsWidget::thawAllSelected() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    QVector<int> indexes;
    for (QModelIndex index: list) {
        indexes.push_back(m_proxyModel->mapToSource(index).row());
    }
    emit thawMulti(indexes);
}

void CoinsWidget::viewOutput() {
    QModelIndex index = ui->coins->currentIndex();

    int row = m_proxyModel->mapToSource(index).row();
    QPointer<CoinsInfo> c;
    m_coins->coin(row, [&c](CoinsInfo &cInfo) {
        c = &cInfo;
    });

    if (c) {
        auto * dialog = new OutputInfoDialog(c, this);
        dialog->show();
    }
}

void CoinsWidget::onSweepOutput() {
    QModelIndex index = ui->coins->currentIndex();
    int row = m_proxyModel->mapToSource(index).row();

    QString keyImage;
    bool keyImageKnown;
    m_coins->coin(row, [&keyImage, &keyImageKnown](CoinsInfo &c) {
        keyImageKnown = c.keyImageKnown();
        keyImage = c.keyImage();
    });

    qCritical() << "key image: " << keyImage;

    if (!keyImageKnown) {
        Utils::showMessageBox("Unable to sweep output", "Unable to sweep output: key image unknown", true);
        return;
    }

    auto * dialog = new OutputSweepDialog(this);
    int ret = dialog->exec();
    if (!ret) return;

    qCritical() << "key image: " << keyImage;

    emit sweepOutput(keyImage, dialog->address(), dialog->churn(), dialog->outputs());
    dialog->deleteLater();
}

void CoinsWidget::copy(copyField field) {
    QModelIndex index = ui->coins->currentIndex();
    int row = m_proxyModel->mapToSource(index).row();

    QString data;
    m_coins->coin(row, [field, &data](CoinsInfo &c) {
        switch (field) {
            case PubKey:
                data = c.pubKey();
                break;
            case KeyImage:
                data = c.keyImage();
                break;
            case TxID:
                data = c.hash();
                break;
            case Address:
                data = c.address();
                break;
            case Height:
                data = QString::number(c.blockHeight());
                break;
            case Amount:
                data = c.displayAmount();
                break;
        }
    });

    Utils::copyToClipboard(data);
}

CoinsWidget::~CoinsWidget() {
    delete ui;
}
