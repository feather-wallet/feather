// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "CoinsWidget.h"
#include "ui_CoinsWidget.h"

#include <QMessageBox>

#include "dialog/OutputInfoDialog.h"
#include "dialog/OutputSweepDialog.h"
#include "utils/Icons.h"

CoinsWidget::CoinsWidget(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CoinsWidget)
        , m_ctx(std::move(ctx))
        , m_headerMenu(new QMenu(this))
        , m_copyMenu(new QMenu("Copy",this))
{
    ui->setupUi(this);

    // header context menu
    ui->coins->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showSpentAction = m_headerMenu->addAction("Show spent outputs", this, &CoinsWidget::setShowSpent);
    m_showSpentAction->setCheckable(true);
    connect(ui->coins->header(), &QHeaderView::customContextMenuRequested, this, &CoinsWidget::showHeaderMenu);

    // copy menu
    m_copyMenu->setIcon(icons()->icon("copy.png"));
    m_copyMenu->addAction("Public key", this, [this]{copy(copyField::PubKey);});
    m_copyMenu->addAction("Key Image", this, [this]{copy(copyField::KeyImage);});
    m_copyMenu->addAction("Transaction ID", this, [this]{copy(copyField::TxID);});
    m_copyMenu->addAction("Address", this, [this]{copy(copyField::Address);});
    m_copyMenu->addAction("Label", this, [this]{copy(copyField::Label);});
    m_copyMenu->addAction("Height", this, [this]{copy(copyField::Height);});
    m_copyMenu->addAction("Amount", this, [this]{copy(copyField::Amount);});

    // context menu
    ui->coins->setContextMenuPolicy(Qt::CustomContextMenu);

    m_thawOutputAction = new QAction("Thaw output", this);
    m_freezeOutputAction = new QAction("Freeze output", this);

    m_freezeAllSelectedAction = new QAction("Freeze selected", this);
    m_thawAllSelectedAction = new QAction("Thaw selected", this);

    m_viewOutputAction = new QAction(icons()->icon("info2.svg"), "Details", this);
    m_sweepOutputAction = new QAction("Sweep output", this);
    m_sweepOutputsAction = new QAction("Sweep selected outputs", this);
    connect(m_freezeOutputAction, &QAction::triggered, this, &CoinsWidget::freezeOutput);
    connect(m_thawOutputAction, &QAction::triggered, this, &CoinsWidget::thawOutput);
    connect(m_viewOutputAction, &QAction::triggered, this, &CoinsWidget::viewOutput);
    connect(m_sweepOutputAction, &QAction::triggered, this, &CoinsWidget::onSweepOutput);
    connect(m_sweepOutputsAction, &QAction::triggered, this, &CoinsWidget::onSweepMulti);

    connect(m_freezeAllSelectedAction, &QAction::triggered, this, &CoinsWidget::freezeAllSelected);
    connect(m_thawAllSelectedAction, &QAction::triggered, this, &CoinsWidget::thawAllSelected);

    connect(ui->coins, &QTreeView::customContextMenuRequested, this, &CoinsWidget::showContextMenu);
    connect(ui->coins, &QTreeView::doubleClicked, [this](QModelIndex index){
       if (!m_model) return;
       if (!(m_model->flags(index) & Qt::ItemIsEditable)) {
           this->viewOutput();
       }
    });

    connect(ui->search, &QLineEdit::textChanged, this, &CoinsWidget::setSearchFilter);
}

void CoinsWidget::setModel(CoinsModel * model, Coins * coins) {
    m_coins = coins;
    m_model = model;
    m_proxyModel = new CoinsProxyModel(this, m_coins);
    m_proxyModel->setSourceModel(m_model);
    ui->coins->setModel(m_proxyModel);
    ui->coins->setColumnHidden(CoinsModel::Spent, true);
    ui->coins->setColumnHidden(CoinsModel::SpentHeight, true);
    ui->coins->setColumnHidden(CoinsModel::Frozen, true);

    if (!m_ctx->wallet->viewOnly()) {
        ui->coins->setColumnHidden(CoinsModel::KeyImageKnown, true);
    } else {
        ui->coins->setColumnHidden(CoinsModel::KeyImageKnown, false);
    }

    ui->coins->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->coins->header()->setSectionResizeMode(CoinsModel::Label, QHeaderView::Stretch);
    ui->coins->header()->setSortIndicator(CoinsModel::BlockHeight, Qt::DescendingOrder);
    ui->coins->setSortingEnabled(true);
}

void CoinsWidget::setSearchbarVisible(bool visible) {
    ui->search->setVisible(visible);
}

void CoinsWidget::showContextMenu(const QPoint &point) {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    auto *menu = new QMenu(ui->coins);
    if (list.size() > 1) {
        menu->addAction(m_freezeAllSelectedAction);
        menu->addAction(m_thawAllSelectedAction);
        menu->addAction(m_sweepOutputsAction);
    }
    else {
        CoinsInfo* c = this->currentEntry();
        if (!c) return;

        bool isSpent = c->spent();
        bool isFrozen = c->frozen();
        bool isUnlocked = c->unlocked();

        menu->addMenu(m_copyMenu);

        if (!isSpent) {
            isFrozen ? menu->addAction(m_thawOutputAction) : menu->addAction(m_freezeOutputAction);

            menu->addAction(m_sweepOutputAction);
            if (isFrozen || !isUnlocked) {
                m_sweepOutputAction->setDisabled(true);
            } else {
                m_sweepOutputAction->setEnabled(true);
            }
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

void CoinsWidget::setSearchFilter(const QString &filter) {
    if (!m_proxyModel) return;
    m_proxyModel->setSearchFilter(filter);
}

void CoinsWidget::freezeOutput() {
    QModelIndex index = ui->coins->currentIndex();
    QVector<int> indexes = {m_proxyModel->mapToSource(index).row()};
    this->freezeCoins(indexes);
}

void CoinsWidget::freezeAllSelected() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    QVector<int> indexes;
    for (QModelIndex index: list) {
        indexes.push_back(m_proxyModel->mapToSource(index).row()); // todo: will segfault if index get invalidated
    }
    this->freezeCoins(indexes);
}

void CoinsWidget::thawOutput() {
    QModelIndex index = ui->coins->currentIndex();
    QVector<int> indexes = {m_proxyModel->mapToSource(index).row()};
    this->thawCoins(indexes);
}

void CoinsWidget::thawAllSelected() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    QVector<int> indexes;
    for (QModelIndex index: list) {
        indexes.push_back(m_proxyModel->mapToSource(index).row());
    }
    this->thawCoins(indexes);
}

void CoinsWidget::viewOutput() {
    CoinsInfo* c = this->currentEntry();
    if (!c) return;

    auto * dialog = new OutputInfoDialog(c, this);
    dialog->show();
}

void CoinsWidget::onSweepOutput() {
    CoinsInfo* c = this->currentEntry();
    if (!c) return;

    QString keyImage = c->keyImage();

    if (!c->keyImageKnown()) {
        QMessageBox::warning(this, "Unable to sweep output", "Unable to sweep output: key image unknown");
        return;
    }

    auto *dialog = new OutputSweepDialog(this, c->amount());
    int ret = dialog->exec();
    if (!ret) return;

    m_ctx->onSweepOutput(keyImage, dialog->address(), dialog->churn(), dialog->outputs());
    dialog->deleteLater();
}

void CoinsWidget::onSweepMulti() {
    QVector<CoinsInfo*> selectedCoins = this->currentEntries();
    QVector<QString> keyImages;

    quint64 totalAmount = 0;
    for (const auto coin : selectedCoins) {
        if (!coin) return;

        QString keyImage = coin->keyImage();
        if (!coin->keyImageKnown()) {
            QMessageBox::warning(this, "Unable to sweep output", "Unable to sweep output: key image unknown");
            return;
        }
        keyImages.push_back(keyImage);
        totalAmount += coin->amount();
    }

    OutputSweepDialog dialog{this, totalAmount};
    int ret = dialog.exec();
    if (!ret) return;

    m_ctx->onSweepOutputs(keyImages, dialog.address(), dialog.churn(), dialog.outputs());
}

void CoinsWidget::copy(copyField field) {
    CoinsInfo* c = this->currentEntry();
    if (!c) return;

    QString data;
    switch (field) {
        case PubKey:
            data = c->pubKey();
            break;
        case KeyImage:
            data = c->keyImage();
            break;
        case TxID:
            data = c->hash();
            break;
        case Address:
            data = c->address();
            break;
        case Label: {
            if (!c->description().isEmpty())
                data = c->description();
            else
                data = c->addressLabel();
            break;
        }
        case Height:
            data = QString::number(c->blockHeight());
            break;
        case Amount:
            data = c->displayAmount();
            break;
    }

    Utils::copyToClipboard(data);
}

CoinsInfo* CoinsWidget::currentEntry() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();
    if (list.size() == 1) {
        return m_model->entryFromIndex(m_proxyModel->mapToSource(list.first()));
    } else {
        return nullptr;
    }
}

QVector<CoinsInfo*> CoinsWidget::currentEntries() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();
    QVector<CoinsInfo*> selectedCoins;
    for (const auto index : list) {
        selectedCoins.push_back(m_model->entryFromIndex(m_proxyModel->mapToSource(index)));
    }
    return selectedCoins;
}

void CoinsWidget::freezeCoins(const QVector<int>& indexes) {
    for (int i : indexes) {
        m_ctx->wallet->coins()->freeze(i);
    }
    m_ctx->wallet->coins()->refresh(m_ctx->wallet->currentSubaddressAccount());
    m_ctx->updateBalance();
}

void CoinsWidget::thawCoins(const QVector<int> &indexes) {
    for (int i : indexes) {
        m_ctx->wallet->coins()->thaw(i);
    }
    m_ctx->wallet->coins()->refresh(m_ctx->wallet->currentSubaddressAccount());
    m_ctx->updateBalance();
}

CoinsWidget::~CoinsWidget() = default;