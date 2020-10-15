// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "debuginfodialog.h"
#include "ui_debuginfodialog.h"
#include "config-feather.h"
#include "utils/utils.h"

#include <QDateTime>
#include <QSysInfo>

DebugInfoDialog::DebugInfoDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::DebugInfoDialog)
{
    ui->setupUi(this);

    QString torStatus;
    if(ctx->isTorSocks)
        torStatus = "Torsocks";
    else if(ctx->tor->localTor)
        torStatus = "Local (assumed to be running)";
    else if(ctx->tor->torConnected)
        torStatus = "Running";
    else
        torStatus = "Unknown";

    ui->label_featherVersion->setText(QString("%1-%2").arg(FEATHER_VERSION, FEATHER_BRANCH));
    ui->label_moneroVersion->setText(QString("%1-%2").arg(MONERO_VERSION, MONERO_BRANCH));

    ui->label_walletHeight->setText(QString::number(ctx->currentWallet->blockChainHeight()));
    ui->label_daemonHeight->setText(QString::number(ctx->currentWallet->daemonBlockChainHeight()));
    ui->label_restoreHeight->setText(QString::number(ctx->currentWallet->getWalletCreationHeight()));
    ui->label_synchronized->setText(ctx->currentWallet->synchronized() ? "True" : "False");

    auto node = ctx->nodes->connection();
    ui->label_remoteNode->setText(node.full);
    ui->label_walletStatus->setText(this->statusToString(ctx->currentWallet->connected()));
    ui->label_torStatus->setText(torStatus);
    ui->label_websocketStatus->setText(Utils::QtEnumToString(ctx->ws->webSocket.state()));

    ui->label_netType->setText(Utils::QtEnumToString(ctx->currentWallet->nettype()));
    ui->label_seedType->setText(ctx->currentWallet->getCacheAttribute("feather.seed").isEmpty() ? "25 word" : "14 word");
    ui->label_viewOnly->setText(ctx->currentWallet->viewOnly() ? "True" : "False");

    QString os = QSysInfo::prettyProductName();
    if (ctx->isTails) {
        os = QString("Tails %1").arg(TailsOS::version());
    }
    ui->label_OS->setText(os);
    ui->label_timestamp->setText(QString::number(QDateTime::currentSecsSinceEpoch()));

    connect(ui->btn_Copy, &QPushButton::clicked, this, &DebugInfoDialog::copyToClipboad);

    this->adjustSize();
}

QString DebugInfoDialog::statusToString(Wallet::ConnectionStatus status) {
    switch (status) {
        case Wallet::ConnectionStatus_Disconnected:
            return "Disconnected";
        case Wallet::ConnectionStatus_Connected:
            return "Connected";
        case Wallet::ConnectionStatus_WrongVersion:
            return "Daemon wrong version";
        case Wallet::ConnectionStatus_Connecting:
            return "Connecting";
        default:
            return "Unknown";
    }
}

void DebugInfoDialog::copyToClipboad() {
    QString text = "";
    text += QString("Feather version: %1\n").arg(ui->label_featherVersion->text());
    text += QString("Monero version: %1\n").arg(ui->label_moneroVersion->text());

    text += QString("Wallet height: %1\n").arg(ui->label_walletHeight->text());
    text += QString("Daemon height: %1\n").arg(ui->label_daemonHeight->text());
    text += QString("Restore height: %1\n").arg(ui->label_restoreHeight->text());
    text += QString("Synchronized: %1\n").arg(ui->label_synchronized->text());

    text += QString("Remote node: %1\n").arg(ui->label_remoteNode->text());
    text += QString("Wallet status: %1\n").arg(ui->label_walletStatus->text());
    text += QString("Tor status: %1\n").arg(ui->label_torStatus->text());
    text += QString("Websocket status: %1\n").arg(ui->label_websocketStatus->text());

    text += QString("Network type: %1\n").arg(ui->label_netType->text());
    text += QString("Seed type: %1\n").arg(ui->label_seedType->text());
    text += QString("View only: %1\n").arg(ui->label_viewOnly->text());

    text += QString("Operating system: %1\n").arg(ui->label_OS->text());
    text += QString("Timestamp: %1\n").arg(ui->label_timestamp->text());

    Utils::copyToClipboard(text);
}

DebugInfoDialog::~DebugInfoDialog() {
    delete ui;
}
