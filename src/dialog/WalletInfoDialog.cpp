// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "WalletInfoDialog.h"
#include "ui_WalletInfoDialog.h"

#include <QDesktopServices>

WalletInfoDialog::WalletInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::WalletInfoDialog)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    QFileInfo cache(m_ctx->wallet->cachePath());

    ui->label_walletName->setText(QFileInfo(m_ctx->wallet->cachePath()).fileName());
    ui->label_netType->setText(Utils::QtEnumToString(m_ctx->wallet->nettype()));
    ui->label_seedType->setText(QString("%1 word").arg(m_ctx->wallet->seedLength()));
    ui->label_viewOnly->setText(m_ctx->wallet->viewOnly() ? "True" : "False");
    ui->label_keysFile->setText(m_ctx->wallet->keysPath());
    ui->label_cacheFile->setText(m_ctx->wallet->cachePath());
    ui->label_cacheSize->setText(QString("%1 MB").arg(QString::number(cache.size() / 1e6, 'f', 2)));

    connect(ui->btn_openWalletDir, &QPushButton::clicked, this, &WalletInfoDialog::openWalletDir);

    ui->label_keysFileLabel->setHelpText("The \"keys file\" stores the wallet keys and wallet settings. "
                                         "It is encrypted with the wallet password (if set).\n\n"
                                         "Your funds will be irreversibly lost if you delete this file "
                                         "without having a backup of your mnemonic seed or private keys.");

    ui->label_cacheFileLabel->setHelpText("The \"cache file\" stores transaction data, contacts, address labels, "
                                          "block hashes, the 14-word seed (if applicable), and other miscellaneous information. "
                                          "It is encrypted with the wallet password (if set).\n\n"
                                          "Warning: Transaction keys and the 14-word seed CANNOT be recovered if this file is deleted.");

    this->adjustSize();
}

void WalletInfoDialog::openWalletDir() {
    QFileInfo file(m_ctx->wallet->keysPath());

    QDesktopServices::openUrl(QUrl(QString("file://%1").arg(file.absolutePath()), QUrl::TolerantMode));
}

WalletInfoDialog::~WalletInfoDialog() = default;