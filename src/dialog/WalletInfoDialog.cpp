// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WalletInfoDialog.h"
#include "ui_WalletInfoDialog.h"

#include <QDesktopServices>

WalletInfoDialog::WalletInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WalletInfoDialog)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    QFileInfo cache(m_ctx->wallet->cachePath());

    ui->label_walletName->setText(QFileInfo(m_ctx->wallet->cachePath()).fileName());
    ui->label_netType->setText(Utils::QtEnumToString(m_ctx->wallet->nettype()));
    ui->label_seedType->setText(m_ctx->wallet->getCacheAttribute("feather.seed").isEmpty() ? "25 word" : "14 word");
    ui->label_viewOnly->setText(m_ctx->wallet->viewOnly() ? "True" : "False");
    ui->label_path->setText(m_ctx->wallet->keysPath());
    ui->label_cacheSize->setText(QString("%1 MB").arg(QString::number(cache.size() / 1e6, 'f', 2)));

    connect(ui->btn_openWalletDir, &QPushButton::clicked, this, &WalletInfoDialog::openWalletDir);

    this->adjustSize();
}

void WalletInfoDialog::openWalletDir() {
    QFileInfo file(m_ctx->wallet->keysPath());

    QDesktopServices::openUrl(QUrl(QString("file://%1").arg(file.absolutePath()), QUrl::TolerantMode));
}

WalletInfoDialog::~WalletInfoDialog() = default;