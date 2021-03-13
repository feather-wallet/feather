// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageOpenWallet.h"
#include "ui_PageOpenWallet.h"

#include <QFileDialog>
#include <QMessageBox>

PageOpenWallet::PageOpenWallet(AppContext *ctx, WalletKeysFilesModel *wallets, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageOpenWallet)
        , m_ctx(ctx)
        , m_walletKeysFilesModel(wallets)
{
    ui->setupUi(this);

    connect(ui->btnBrowse, &QPushButton::clicked, [=]{
        // manually browsing for wallet
        auto walletPath = config()->get(Config::walletPath).toString();
        if (walletPath.isEmpty())
            walletPath = m_ctx->defaultWalletDir;
        QString path = QFileDialog::getOpenFileName(this, "Select your wallet file", walletPath, "Wallet file (*.keys)");
        if(path.isEmpty()) return;

        QFileInfo infoPath(path);
        if(!infoPath.isReadable()) {
            QMessageBox::warning(this, "Cannot read wallet file", "Permission error.");
            return;
        }

        if (ui->openOnStartup->isChecked())
            config()->set(Config::autoOpenWalletPath, QString("%1%2").arg(m_ctx->networkType).arg(path));

        emit openWallet(path);
    });

    this->setTitle("Open wallet file");
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    ui->walletTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->walletTable->setContextMenuPolicy(Qt::CustomContextMenu);

    m_keysProxy = new WalletKeysFilesProxyModel(this, m_ctx->networkType);
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
    connect(ui->walletTable, &QTreeView::doubleClicked, this, &PageOpenWallet::validatePage);
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

    auto autoWallet = ui->openOnStartup->isChecked() ? QString("%1%2").arg(m_ctx->networkType).arg(walletPath) : "";
    config()->set(Config::autoOpenWalletPath, autoWallet);

    emit openWallet(walletPath);
    return true;
}
