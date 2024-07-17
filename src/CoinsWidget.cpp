// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "CoinsWidget.h"
#include "ui_CoinsWidget.h"

#include <QMessageBox>

#include "dialog/OutputInfoDialog.h"
#include "dialog/OutputSweepDialog.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

#ifdef WITH_SCANNER
#include "wizard/offline_tx_signing/OfflineTxSigningWizard.h"
#endif

CoinsWidget::CoinsWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CoinsWidget)
        , m_wallet(wallet)
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
    m_copyMenu->addAction("Public Key", this, [this]{copy(copyField::PubKey);});
    m_copyMenu->addAction("Key Image", this, [this]{copy(copyField::KeyImage);});
    m_copyMenu->addAction("Transaction ID", this, [this]{copy(copyField::TxID);});
    m_copyMenu->addAction("Address", this, [this]{copy(copyField::Address);});
    m_copyMenu->addAction("Label", this, [this]{copy(copyField::Label);});
    m_copyMenu->addAction("Height", this, [this]{copy(copyField::Height);});
    m_copyMenu->addAction("Amount", this, [this]{copy(copyField::Amount);});

    // context menu
    ui->coins->setContextMenuPolicy(Qt::CustomContextMenu);

    m_editLabelAction = new QAction("Edit Label", this);
    connect(m_editLabelAction, &QAction::triggered, this, &CoinsWidget::editLabel);

    m_thawOutputAction = new QAction("Thaw output", this);
    m_freezeOutputAction = new QAction("Freeze output", this);

    m_freezeAllSelectedAction = new QAction("Freeze selected", this);
    m_thawAllSelectedAction = new QAction("Thaw selected", this);

    m_spendAction = new QAction("Spend", this);
    m_viewOutputAction = new QAction("Details", this);
    m_sweepOutputAction = new QAction("Sweep output", this);
    m_sweepOutputsAction = new QAction("Sweep selected outputs", this);

    connect(m_freezeOutputAction, &QAction::triggered, this, &CoinsWidget::freezeAllSelected);
    connect(m_thawOutputAction, &QAction::triggered, this, &CoinsWidget::thawAllSelected);
    connect(m_spendAction, &QAction::triggered, this, &CoinsWidget::spendSelected);
    connect(m_viewOutputAction, &QAction::triggered, this, &CoinsWidget::viewOutput);
    connect(m_sweepOutputAction, &QAction::triggered, this, &CoinsWidget::onSweepOutputs);
    connect(m_sweepOutputsAction, &QAction::triggered, this, &CoinsWidget::onSweepOutputs);

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

    connect(m_wallet, &Wallet::selectedInputsChanged, this, &CoinsWidget::selectCoins);
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

    bool showKeyImageKnown = m_wallet->viewOnly() || m_wallet->isMultisig();
    ui->coins->setColumnHidden(CoinsModel::KeyImageKnown, !showKeyImageKnown);

    ui->coins->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->coins->header()->setSectionResizeMode(CoinsModel::Label, QHeaderView::Stretch);
    ui->coins->header()->setSortIndicator(CoinsModel::BlockHeight, Qt::DescendingOrder);
    ui->coins->setSortingEnabled(true);
}

void CoinsWidget::setSearchbarVisible(bool visible) {
    ui->search->setVisible(visible);
}

void CoinsWidget::focusSearchbar() {
    ui->search->setFocusPolicy(Qt::StrongFocus);
    ui->search->setFocus();
}

void CoinsWidget::showContextMenu(const QPoint &point) {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    auto *menu = new QMenu(ui->coins);
    if (list.size() > 1) {
        menu->addAction(m_spendAction);
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

        menu->addAction(m_spendAction);
        menu->addMenu(m_copyMenu);
        menu->addAction(m_editLabelAction);

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

QStringList CoinsWidget::selectedPubkeys() {
    QModelIndexList list = ui->coins->selectionModel()->selectedRows();

    QStringList pubkeys;
    for (QModelIndex index: list) {
        pubkeys << m_model->entryFromIndex(m_proxyModel->mapToSource(index))->pubKey();
    }
    return pubkeys;
}

void CoinsWidget::freezeAllSelected() {
    QStringList pubkeys = this->selectedPubkeys();
    this->freezeCoins(pubkeys);
}

void CoinsWidget::thawAllSelected() {
    QStringList pubkeys = this->selectedPubkeys();
    this->thawCoins(pubkeys);
}

void CoinsWidget::spendSelected() {
    QVector<CoinsInfo*> selectedCoins = this->currentEntries();
    QStringList keyimages;

    for (const auto coin : selectedCoins) {
        if (!coin) return;

        bool spendable = this->isCoinSpendable(coin);
        if (!spendable) return;

        QString keyImage = coin->keyImage();
        keyimages << keyImage;
    }

    m_wallet->setSelectedInputs(keyimages);
    this->selectCoins(keyimages);
}

void CoinsWidget::viewOutput() {
    CoinsInfo* c = this->currentEntry();
    if (!c) return;

    auto * dialog = new OutputInfoDialog(c, this);
    dialog->show();
}

void CoinsWidget::onSweepOutputs() {
    QVector<CoinsInfo*> selectedCoins = this->currentEntries();
    QVector<QString> keyImages;

    quint64 totalAmount = 0;
    for (const auto coin : selectedCoins) {
        if (!coin) return;

        bool spendable = this->isCoinSpendable(coin);
        if (!spendable) return;

        QString keyImage = coin->keyImage();
        keyImages.push_back(keyImage);
        totalAmount += coin->amount();
    }

    OutputSweepDialog dialog{this, totalAmount};
    int ret = dialog.exec();
    if (!ret) return;

    if (m_wallet->keyImageSyncNeeded(totalAmount, false)) {
#if defined(WITH_SCANNER)
        OfflineTxSigningWizard wizard(this, m_wallet);
        auto r = wizard.exec();
    
        if (r == QDialog::Rejected) {
            return;
        }
#else
        Utils::showError(this, "Can't open offline transaction signing wizard", "Feather was built without webcam QR scanner support");
        return;
#endif
    }

    m_wallet->sweepOutputs(keyImages, dialog.address(), dialog.churn(), dialog.outputs());
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

void CoinsWidget::freezeCoins(QStringList &pubkeys) {
    for (auto &pubkey : pubkeys) {
        m_wallet->coins()->freeze(pubkey);
    }
    m_wallet->coins()->refresh();
    m_wallet->updateBalance();
}

void CoinsWidget::thawCoins(QStringList &pubkeys) {
    for (auto &pubkey : pubkeys) {
        m_wallet->coins()->thaw(pubkey);
    }
    m_wallet->coins()->refresh();
    m_wallet->updateBalance();
}

void CoinsWidget::selectCoins(const QStringList &keyimages) {
    m_model->setSelected(keyimages);
    ui->coins->clearSelection();
}

void CoinsWidget::editLabel() {
    QModelIndex index = ui->coins->currentIndex().siblingAtColumn(m_model->ModelColumn::Label);
    ui->coins->setCurrentIndex(index);
    ui->coins->edit(index);
}

bool CoinsWidget::isCoinSpendable(CoinsInfo *coin) {
    if (!coin->keyImageKnown()) {
        Utils::showError(this, "Unable to spend outputs", "Selected output has unknown key image");
        return false;
    }

    if (!coin->haveMultisigK()) {
        Utils::showError(this, "Unable to spend outputs", "We already spent this output in another transaction proposal, other participants would not be able to sign both transactions");
        return false;
    }

    if (coin->spent()) {
        Utils::showError(this, "Unable to spend outputs", "Selected output was already spent");
        return false;
    }

    if (coin->frozen()) {
        Utils::showError(this, "Unable to spend outputs", "Selected output is frozen", {"Thaw the selected output(s) before spending"}, "freeze_thaw_outputs");
        return false;
    }

    if (!coin->unlocked()) {
        Utils::showError(this, "Unable to spend outputs", "Selected output is locked", {"Wait until the output has reached the required number of confirmation before spending."});
        return false;
    }

    return true;
}

CoinsWidget::~CoinsWidget() = default;