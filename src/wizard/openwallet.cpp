// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "wizard/openwallet.h"
#include "ui_openwallet.h"
#include "appcontext.h"
#include "utils/config.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

// @TODO: rescan wallet dir on wizard open

OpenWalletPage::OpenWalletPage(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        ui(new Ui::OpenWalletPage),
        m_ctx(ctx) {
    ui->setupUi(this);

    connect(ui->btnBrowse, &QPushButton::clicked, [=]{
        // manually browsing for wallet
        auto walletPath = config()->get(Config::walletPath).toString();
        if(walletPath.isEmpty())
            walletPath = m_ctx->defaultWalletDir;
        QString path = QFileDialog::getOpenFileName(this, "Select your wallet file", walletPath, "Wallet file (*.keys)");
        if(path.isEmpty()) return;

        QFileInfo infoPath(path);
        if(!infoPath.isReadable()) {
            QMessageBox::warning(this, "Cannot read wallet file", "Permission error.");
            return;
        }

        setField("walletPath", path);

        if(ui->openOnStartup->isChecked())
            config()->set(Config::autoOpenWalletPath, QString("%1%2").arg(m_ctx->networkType).arg(path));

        emit openWallet(path);
    });

    this->setTitle("Open wallet file");
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    ui->walletTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->walletTable->setContextMenuPolicy(Qt::CustomContextMenu);

    this->walletKeysFilesModel = new WalletKeysFilesModel(m_ctx);
    this->walletKeysFilesModel->refresh();

    m_keysProxy = new WalletKeysFilesProxyModel(this, m_ctx->networkType);
    m_keysProxy->setSourceModel(this->walletKeysFilesModel);
    m_keysProxy->setSortRole(Qt::UserRole);

    ui->walletTable->setModel(m_keysProxy);
    ui->walletTable->hideColumn(WalletKeysFilesModel::NetworkType);
    ui->walletTable->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->walletTable->header()->setSectionResizeMode(WalletKeysFilesModel::Path, QHeaderView::Stretch);
    ui->walletTable->setSortingEnabled(true);
    ui->walletTable->sortByColumn(WalletKeysFilesModel::Modified, Qt::AscendingOrder); // @TODO: this does not work
    ui->walletTable->show();
}

int OpenWalletPage::nextId() const {
    return -1;
}

bool OpenWalletPage::validatePage() {
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
