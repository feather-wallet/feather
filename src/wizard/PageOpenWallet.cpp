// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageOpenWallet.h"
#include "ui_PageOpenWallet.h"

#include <QFileDialog>
#include <QMessageBox>

#include "constants.h"
#include "WalletWizard.h"

PageOpenWallet::PageOpenWallet(WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageOpenWallet)
        , m_walletKeysFilesModel(wallets)
{
    ui->setupUi(this);

    connect(ui->btnBrowse, &QPushButton::clicked, [this]{
        QString walletDir = config()->get(Config::walletDirectory).toString();
        QString path = QFileDialog::getOpenFileName(this, "Select your wallet file", walletDir, "Wallet file (*.keys)");
        if (path.isEmpty())
            return;

        QFileInfo infoPath(path);
        if(!infoPath.isReadable()) {
            QMessageBox::warning(this, "Cannot read wallet file", "Permission error.");
            return;
        }

        if (ui->openOnStartup->isChecked())
            config()->set(Config::autoOpenWalletPath, QString("%1%2").arg(constants::networkType).arg(path));

        emit openWallet(path);
    });

    this->setTitle("Open wallet file");
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    ui->walletTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->walletTable->setContextMenuPolicy(Qt::CustomContextMenu);

    m_keysProxy = new WalletKeysFilesProxyModel(this, constants::networkType);
    m_keysProxy->setSourceModel(m_walletKeysFilesModel);
    m_keysProxy->setSortRole(Qt::UserRole);

    ui->walletTable->setModel(m_keysProxy);
    ui->walletTable->hideColumn(WalletKeysFilesModel::NetworkType);
    ui->walletTable->hideColumn(WalletKeysFilesModel::Path);
    ui->walletTable->hideColumn(WalletKeysFilesModel::Modified);
    ui->walletTable->header()->setSectionResizeMode(WalletKeysFilesModel::FileName, QHeaderView::Stretch);
    ui->walletTable->setSortingEnabled(true);
    ui->walletTable->sortByColumn(WalletKeysFilesModel::Modified, Qt::DescendingOrder);
    ui->walletTable->show();

    connect(ui->walletTable->selectionModel(), &QItemSelectionModel::currentRowChanged, [this](QModelIndex current, QModelIndex prev){
        this->updatePath();
    });
    connect(ui->walletTable, &QTreeView::doubleClicked, [this]{
        // Simulate next button click
        QWizard *wizard = this->wizard();
        if (wizard) {
            wizard->button(QWizard::FinishButton)->click();
        }
    });
}

void PageOpenWallet::initializePage() {
    m_walletKeysFilesModel->refresh();
}

void PageOpenWallet::updatePath() {
    QModelIndex index = ui->walletTable->currentIndex();
    if (!index.isValid()) {
        ui->linePath->clear();
        return;
    }

    QString path = index.model()->data(index.siblingAtColumn(WalletKeysFilesModel::Path), Qt::DisplayRole).toString();
    ui->linePath->setText(path);
}

int PageOpenWallet::nextId() const {
    return -1;
}

bool PageOpenWallet::validatePage() {
    QModelIndex index = ui->walletTable->currentIndex();
    if(!index.isValid()) {
        QMessageBox::warning(this, "Wallet not selected", "Please select a wallet from the list.");
        return false;
    }
    QString walletPath = index.model()->data(index.siblingAtColumn(WalletKeysFilesModel::ModelColumns::Path), Qt::UserRole).toString();

    auto autoWallet = ui->openOnStartup->isChecked() ? QString("%1%2").arg(constants::networkType).arg(walletPath) : "";
    config()->set(Config::autoOpenWalletPath, autoWallet);

    emit openWallet(walletPath);
    return true;
}
