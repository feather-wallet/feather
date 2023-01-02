// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

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

    this->setTitle("Open wallet file");
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    m_keysProxy = new WalletKeysFilesProxyModel(this, constants::networkType);
    m_keysProxy->setSourceModel(m_walletKeysFilesModel);
    m_keysProxy->setSortRole(Qt::UserRole);

    ui->walletTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->walletTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->walletTable->setModel(m_keysProxy);
    ui->walletTable->hideColumn(WalletKeysFilesModel::NetworkType);
    ui->walletTable->hideColumn(WalletKeysFilesModel::Path);
    ui->walletTable->hideColumn(WalletKeysFilesModel::Modified);
    ui->walletTable->header()->setSectionResizeMode(WalletKeysFilesModel::FileName, QHeaderView::Stretch);
    ui->walletTable->setSortingEnabled(true);
    ui->walletTable->sortByColumn(WalletKeysFilesModel::Modified, Qt::DescendingOrder);

    connect(ui->walletTable->selectionModel(), &QItemSelectionModel::currentRowChanged, [this](QModelIndex current, QModelIndex prev){
        this->updatePath();
    });
    connect(ui->walletTable, &QTreeView::doubleClicked, this, &PageOpenWallet::nextPage);

    connect(ui->btnBrowse, &QPushButton::clicked, [this]{
        QString walletDir = config()->get(Config::walletDirectory).toString();
        m_walletFile = QFileDialog::getOpenFileName(this, "Select your wallet file", walletDir, "Wallet file (*.keys)");
        if (m_walletFile.isEmpty())
            return;
        this->nextPage();
    });

    this->updatePath();
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

    m_walletFile = index.model()->data(index.siblingAtColumn(WalletKeysFilesModel::Path), Qt::UserRole).toString();
    QString path = index.model()->data(index.siblingAtColumn(WalletKeysFilesModel::Path), Qt::DisplayRole).toString();
    ui->linePath->setText(path);
}

int PageOpenWallet::nextId() const {
    return -1;
}

void PageOpenWallet::nextPage() {
    // Simulate next button click
    QWizard *wizard = this->wizard();
    if (wizard) {
        wizard->button(QWizard::FinishButton)->click();
    }
}

bool PageOpenWallet::validatePage() {
    if (m_walletFile.isEmpty()) {
        QMessageBox::warning(this, "No wallet file selected", "Please select a wallet from the list.");
        return false;
    }

    QFileInfo infoPath(m_walletFile);
    if (!infoPath.isReadable()) {
        QMessageBox::warning(this, "Permission error", "Cannot read wallet file.");
        return false;
    }

    // Clear autoOpen if openOnStartup is not checked
    auto autoWallet = ui->openOnStartup->isChecked() ? QString("%1%2").arg(constants::networkType).arg(m_walletFile) : "";
    config()->set(Config::autoOpenWalletPath, autoWallet);

    emit openWallet(m_walletFile);
    return true;
}
