// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "DebugInfoDialog.h"
#include "ui_DebugInfoDialog.h"

#include "config-feather.h"
#include "utils/os/tails.h"
#include "utils/TorManager.h"
#include "utils/WebsocketClient.h"
#include "utils/WebsocketNotifier.h"

DebugInfoDialog::DebugInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::DebugInfoDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    connect(ui->btn_Copy, &QPushButton::clicked, this, &DebugInfoDialog::copyToClipboad);

    m_updateTimer.start(5000);
    connect(&m_updateTimer, &QTimer::timeout, this, &DebugInfoDialog::updateInfo);
    this->updateInfo();

    this->adjustSize();
}

void DebugInfoDialog::updateInfo() {
    QString torStatus;

    // Special case for Tails because we know the status of the daemon by polling tails-tor-has-bootstrapped.target
    if (TailsOS::detect()) {
        if(torManager()->torConnected)
            torStatus = "Connected";
        else
            torStatus = "Disconnected";
    }
    else if(Utils::isTorsocks())
        torStatus = "Torsocks";
    else if(torManager()->isLocalTor())
        torStatus = "Local (assumed to be running)";
    else if(torManager()->torConnected)
        torStatus = "Running";
    else
        torStatus = "Unknown";

    ui->label_featherVersion->setText(QString("%1-%2").arg(FEATHER_VERSION, FEATHER_COMMIT));

    ui->label_walletHeight->setText(QString::number(m_ctx->wallet->blockChainHeight()));
    ui->label_daemonHeight->setText(QString::number(m_ctx->wallet->daemonBlockChainHeight()));
    ui->label_targetHeight->setText(QString::number(m_ctx->wallet->daemonBlockChainTargetHeight()));
    QDateTime restoreDate = appData()->restoreHeights[constants::networkType]->heightToDate(m_ctx->wallet->getWalletCreationHeight());
    ui->label_restoreHeight->setText(QString("%1 (%2)").arg(QString::number(m_ctx->wallet->getWalletCreationHeight()), restoreDate.toString("yyyy-MM-dd")));
    ui->label_synchronized->setText(m_ctx->wallet->isSynchronized() ? "True" : "False");

    auto node = m_ctx->nodes->connection();
    ui->label_remoteNode->setText(node.toAddress());
    ui->label_walletStatus->setText(this->statusToString(m_ctx->wallet->connectionStatus()));
    QString websocketStatus = Utils::QtEnumToString(websocketNotifier()->websocketClient.webSocket.state()).remove("State");
    if (config()->get(Config::disableWebsocket).toBool()) {
        websocketStatus = "Disabled";
    }
    ui->label_websocketStatus->setText(websocketStatus);
    ui->label_torStatus->setText(torStatus);
    ui->label_torLevel->setText(config()->get(Config::torPrivacyLevel).toString());

    QString seedType = [this](){
        if (m_ctx->wallet->isHwBacked())
            return QString("Hardware");
        return QString("%1 word").arg(m_ctx->wallet->seedLength());
    }();

    QString deviceType = [this](){
        if (m_ctx->wallet->isHwBacked()) {
            if (m_ctx->wallet->isLedger())
                return "Ledger";
            else if (m_ctx->wallet->isTrezor())
                return "Trezor";
            else
                return "Unknown";
        }
        else {
            return "Software";
        }
    }();

    QString networkType = Utils::QtEnumToString(m_ctx->wallet->nettype());
    if (config()->get(Config::offlineMode).toBool()) {
        networkType += " (offline)";
    }
    ui->label_netType->setText(networkType);
    ui->label_seedType->setText(seedType);
    ui->label_deviceType->setText(deviceType);
    ui->label_viewOnly->setText(m_ctx->wallet->viewOnly() ? "True" : "False");
    ui->label_primaryOnly->setText(m_ctx->wallet->balance(0) == m_ctx->wallet->balanceAll() ? "True" : "False");

    QString os = QSysInfo::prettyProductName();
    if (TailsOS::detect()) {
        os = QString("Tails %1").arg(TailsOS::version());
    }
    if (WhonixOS::detect()) {
        os = QString("Whonix %1").arg(WhonixOS::version());
    }
    ui->label_OS->setText(os);
    ui->label_timestamp->setText(QString::number(QDateTime::currentSecsSinceEpoch()));
}

QString DebugInfoDialog::statusToString(Wallet::ConnectionStatus status) {
    switch (status) {
        case Wallet::ConnectionStatus_Disconnected:
            return "Disconnected";
        case Wallet::ConnectionStatus_WrongVersion:
            return "Daemon wrong version";
        case Wallet::ConnectionStatus_Connecting:
            return "Connecting";
        case Wallet::ConnectionStatus_Synchronizing:
            return "Synchronizing";
        case Wallet::ConnectionStatus_Synchronized:
            return "Synchronized";
        default:
            return "Unknown";
    }
}

void DebugInfoDialog::copyToClipboad() {
    // Two spaces at the end of each line are for newlines in Markdown
    QString text = "";
    text += QString("Feather version: %1  \n").arg(ui->label_featherVersion->text());

    text += QString("Wallet height: %1  \n").arg(ui->label_walletHeight->text());
    text += QString("Daemon height: %1  \n").arg(ui->label_daemonHeight->text());
    text += QString("Target height: %1  \n").arg(ui->label_targetHeight->text());
    text += QString("Restore height: %1  \n").arg(ui->label_restoreHeight->text());
    text += QString("Synchronized: %1  \n").arg(ui->label_synchronized->text());

    text += QString("Remote node: %1  \n").arg(ui->label_remoteNode->text());
    text += QString("Wallet status: %1  \n").arg(ui->label_walletStatus->text());
    text += QString("Websocket status: %1  \n").arg(ui->label_websocketStatus->text());
    text += QString("Tor status: %1  \n").arg(ui->label_torStatus->text());
    text += QString("Tor level: %1  \n").arg(ui->label_torLevel->text());

    text += QString("Network type: %1  \n").arg(ui->label_netType->text());
    text += QString("Seed type: %1  \n").arg(ui->label_seedType->text());
    text += QString("Device type: %1  \n").arg(ui->label_deviceType->text());
    text += QString("View only: %1  \n").arg(ui->label_viewOnly->text());
    text += QString("Primary only: %1  \n").arg(ui->label_primaryOnly->text());

    text += QString("Operating system: %1  \n").arg(ui->label_OS->text());
    text += QString("Timestamp: %1  \n").arg(ui->label_timestamp->text());

    Utils::copyToClipboard(text);
}

DebugInfoDialog::~DebugInfoDialog() = default;