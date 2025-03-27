// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AccountSwitcherDialog.h"
#include "ui_AccountSwitcherDialog.h"

#include <QMenu>

#include "libwalletqt/SubaddressAccount.h"
#include "libwalletqt/WalletManager.h"
#include "model/SubaddressAccountModel.h"
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

    connect(ui->accounts, &QTreeView::clicked, this, &AccountSwitcherDialog::switchAccount);
    connect(ui->accounts, &QTreeView::customContextMenuRequested, this, &AccountSwitcherDialog::showContextMenu);

    connect(ui->btn_newAccount, &QPushButton::clicked, [this]{
        m_wallet->addSubaddressAccount("New account");
        m_wallet->subaddressAccount()->refresh();
        for (int i = 0; i < 10; i ++) {
            m_wallet->subaddress()->addRow("");
        }
    });

    connect(m_wallet->subaddressAccount(), &SubaddressAccount::refreshFinished, this, &AccountSwitcherDialog::updateSelection);
}

void AccountSwitcherDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    this->update();
    m_wallet->switchSubaddressAccount(m_wallet->currentSubaddressAccount());
}

void AccountSwitcherDialog::update() {
    ui->label_totalBalance->setText(WalletManager::displayAmount(m_wallet->balanceAll()));
    m_wallet->subaddressAccount()->refresh();
}

void AccountSwitcherDialog::switchAccount(const QModelIndex &index) {
    m_wallet->switchSubaddressAccount(m_proxyModel->mapToSource(index).row());
}

void AccountSwitcherDialog::copyLabel() {
    QModelIndex index = m_proxyModel->mapToSource(ui->accounts->currentIndex());
    if (!index.isValid()) {
        return;
    }

    auto& row = m_wallet->subaddressAccountModel()->entryFromIndex(index);

    Utils::copyToClipboard(row.label);
}

void AccountSwitcherDialog::copyBalance() {
    QModelIndex index = m_proxyModel->mapToSource(ui->accounts->currentIndex());
    if (!index.isValid()) {
        return;
    }

    auto& row = m_wallet->subaddressAccountModel()->entryFromIndex(index);

    Utils::copyToClipboard(WalletManager::displayAmount(row.balance));
}

void AccountSwitcherDialog::editLabel() {
    QModelIndex index = ui->accounts->currentIndex().siblingAtColumn(m_wallet->subaddressAccountModel()->Column::Label);
    ui->accounts->setCurrentIndex(index);
    ui->accounts->edit(index);
}

void AccountSwitcherDialog::updateSelection() {
    QModelIndex index = m_proxyModel->index(m_wallet->currentSubaddressAccount(), 0);
    if (!index.isValid()) {
        return;
    }

    ui->accounts->setCurrentIndex(index);
    ui->accounts->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
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

AccountSwitcherDialog::~AccountSwitcherDialog() = default;
