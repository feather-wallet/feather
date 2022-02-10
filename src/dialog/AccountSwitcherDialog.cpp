// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "AccountSwitcherDialog.h"
#include "ui_AccountSwitcherDialog.h"

#include <QMenu>

#include "libwalletqt/SubaddressAccount.h"
#include "model/ModelUtils.h"
#include "utils/Icons.h"

AccountSwitcherDialog::AccountSwitcherDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::AccountSwitcherDialog)
    , m_ctx(std::move(ctx))
    , m_model(m_ctx->wallet->subaddressAccountModel())
    , m_proxyModel(new SubaddressAccountProxyModel(this))
{
    ui->setupUi(this);

    m_ctx->wallet->subaddressAccount()->refresh();
    m_proxyModel->setSourceModel(m_model);

    ui->label_totalBalance->setFont(ModelUtils::getMonospaceFont());
    ui->label_totalBalance->setText(WalletManager::displayAmount(m_ctx->wallet->balanceAll()));

    this->setWindowModality(Qt::WindowModal);

    ui->accounts->setModel(m_proxyModel);
    ui->accounts->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->accounts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->accounts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->accounts->setSortingEnabled(true);
    ui->accounts->sortByColumn(SubaddressAccountModel::Column::Number, Qt::AscendingOrder);
    ui->accounts->hideColumn(SubaddressAccountModel::Column::Address);
    ui->accounts->hideColumn(SubaddressAccountModel::Column::UnlockedBalance);
    ui->accounts->setColumnWidth(SubaddressAccountModel::Column::Label, 200);
    ui->accounts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->accounts->header()->setSectionResizeMode(SubaddressAccountModel::Label, QHeaderView::Stretch);

    connect(ui->accounts->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountSwitcherDialog::switchAccount);
    connect(ui->accounts, &QTreeView::customContextMenuRequested, this, &AccountSwitcherDialog::showContextMenu);

    connect(ui->btn_newAccount, &QPushButton::clicked, [this]{
       m_ctx->wallet->addSubaddressAccount("New account");
       m_ctx->wallet->subaddressAccount()->refresh();
    });

    connect(m_ctx->wallet, &Wallet::currentSubaddressAccountChanged, this, &AccountSwitcherDialog::updateSelection);
    connect(m_ctx->wallet->subaddressAccount(), &SubaddressAccount::refreshFinished, this, &AccountSwitcherDialog::updateSelection);

    this->updateSelection();
}

void AccountSwitcherDialog::switchAccount() {
    QModelIndex index = ui->accounts->currentIndex();
    m_ctx->wallet->switchSubaddressAccount(index.row());
}

void AccountSwitcherDialog::copyLabel() {
    auto row = this->currentEntry();
    if (!row)
        return;

    Utils::copyToClipboard(QString::fromStdString(row->getLabel()));
}

void AccountSwitcherDialog::copyBalance() {
    auto row = this->currentEntry();
    if (!row)
        return;

    Utils::copyToClipboard(QString::fromStdString(row->getBalance()));
}

void AccountSwitcherDialog::editLabel() {
    QModelIndex index = ui->accounts->currentIndex().siblingAtColumn(m_ctx->wallet->subaddressAccountModel()->Column::Label);
    ui->accounts->setCurrentIndex(index);
    ui->accounts->edit(index);
}

void AccountSwitcherDialog::updateSelection() {
    QModelIndex index = m_model->index(m_ctx->wallet->currentSubaddressAccount(), 0);
    ui->accounts->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void AccountSwitcherDialog::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->accounts->currentIndex();
    if (!index.isValid()) {
        return;
    }

    auto *menu = new QMenu(ui->accounts);

    menu->addAction("Copy label", this, &AccountSwitcherDialog::copyLabel);
    menu->addAction("Copy balance", this, &AccountSwitcherDialog::copyBalance);
    menu->addAction("Edit label", this, &AccountSwitcherDialog::editLabel);

    menu->popup(ui->accounts->viewport()->mapToGlobal(point));
}

Monero::SubaddressAccountRow* AccountSwitcherDialog::currentEntry() {
    QModelIndex index = ui->accounts->currentIndex();
    return m_ctx->wallet->subaddressAccountModel()->entryFromIndex(index);
}

AccountSwitcherDialog::~AccountSwitcherDialog() = default;
