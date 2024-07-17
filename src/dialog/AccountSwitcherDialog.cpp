// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AccountSwitcherDialog.h"
#include "ui_AccountSwitcherDialog.h"

#include <QMenu>

#include "libwalletqt/SubaddressAccount.h"
#include "libwalletqt/WalletManager.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

AccountSwitcherDialog::AccountSwitcherDialog(Wallet *wallet, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::AccountSwitcherDialog)
    , m_wallet(wallet)
    , m_model(wallet->subaddressAccountModel())
    , m_proxyModel(new SubaddressAccountProxyModel(this))
{
    ui->setupUi(this);

    m_proxyModel->setSourceModel(m_model);

    ui->label_totalBalance->setFont(Utils::getMonospaceFont());

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
       m_wallet->addSubaddressAccount("New account");
       m_wallet->subaddressAccount()->refresh();
    });

    connect(m_wallet, &Wallet::currentSubaddressAccountChanged, this, &AccountSwitcherDialog::updateSelection);
    connect(m_wallet->subaddressAccount(), &SubaddressAccount::refreshFinished, this, &AccountSwitcherDialog::updateSelection);

    this->update();
//    this->updateSelection();
}

void AccountSwitcherDialog::update() {
    ui->label_totalBalance->setText(WalletManager::displayAmount(m_wallet->balanceAll()));
    m_wallet->subaddressAccount()->refresh();
}

void AccountSwitcherDialog::switchAccount() {
    auto row = this->currentEntry();
    if (!row) {
        return;
    }

    m_wallet->switchSubaddressAccount(row->getRow());
}

void AccountSwitcherDialog::copyLabel() {
    auto row = this->currentEntry();
    if (!row) {
        return;
    }

    Utils::copyToClipboard(row->getLabel());
}

void AccountSwitcherDialog::copyBalance() {
    auto row = this->currentEntry();
    if (!row) {
        return;
    }

    Utils::copyToClipboard(row->getBalance());
}

void AccountSwitcherDialog::editLabel() {
    QModelIndex index = ui->accounts->currentIndex().siblingAtColumn(m_wallet->subaddressAccountModel()->Column::Label);
    ui->accounts->setCurrentIndex(index);
    ui->accounts->edit(index);
}

void AccountSwitcherDialog::updateSelection() {
    QModelIndex index = m_model->index(m_wallet->currentSubaddressAccount(), 0);
    if (!index.isValid()) {
        return;
    }
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

AccountRow* AccountSwitcherDialog::currentEntry() {
    QModelIndex index = m_proxyModel->mapToSource(ui->accounts->currentIndex());
    return m_wallet->subaddressAccountModel()->entryFromIndex(index);
}

AccountSwitcherDialog::~AccountSwitcherDialog() = default;
