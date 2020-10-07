// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "walletinfodialog.h"
#include "ui_walletinfodialog.h"

#include <QDesktopServices>

WalletInfoDialog::WalletInfoDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::WalletInfoDialog)
        , m_ctx(ctx)
{
    ui->setupUi(this);

    QFileInfo keys(ctx->walletPath);
    QFileInfo cache(ctx->currentWallet->path());

    ui->label_walletName->setText(keys.fileName().replace(".keys", ""));
    ui->label_netType->setText(Utils::QtEnumToString(ctx->currentWallet->nettype()));
    ui->label_seedType->setText(ctx->currentWallet->getCacheAttribute("feather.seed").isEmpty() ? "25 word" : "14 word");
    ui->label_viewOnly->setText(ctx->currentWallet->viewOnly() ? "True" : "False");
    ui->label_path->setText(ctx->walletPath);
    ui->label_cacheSize->setText(QString("%1 MB").arg(QString::number(cache.size() / 1e6, 'f', 2)));

    connect(ui->btn_openWalletDir, &QPushButton::clicked, this, &WalletInfoDialog::openWalletDir);

    this->adjustSize();
}

void WalletInfoDialog::openWalletDir() {
    QFileInfo file(m_ctx->walletPath);

    QDesktopServices::openUrl(file.absolutePath());
}

WalletInfoDialog::~WalletInfoDialog() {
    delete ui;
}
