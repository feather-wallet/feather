// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TorInfoDialog.h"
#include "ui_TorInfoDialog.h"

#include "utils/TorManager.h"

TorInfoDialog::TorInfoDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TorInfoDialog)
{
    ui->setupUi(this);

    ui->logs->setPlainText(torManager()->torLogs);

    this->onStatusChanged(torManager()->errorMsg);
    this->onConnectionStatusChanged(torManager()->torConnected);

    connect(torManager(), &TorManager::statusChanged, this, &TorInfoDialog::onStatusChanged);
    connect(torManager(), &TorManager::connectionStateChanged, this, &TorInfoDialog::onConnectionStatusChanged);
    connect(torManager(), &TorManager::logsUpdated, this, &TorInfoDialog::onLogsUpdated);

    this->adjustSize();
}

void TorInfoDialog::onLogsUpdated() {
    ui->logs->setPlainText(torManager()->torLogs);
}

void TorInfoDialog::onConnectionStatusChanged(bool connected) {
    if (!torManager()->isStarted()) {
        ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_offline.svg").scaledToWidth(16, Qt::SmoothTransformation));
        ui->label_testConnectionStatus->setText("Not running");
    }
    else if (connected) {
        ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_connected.png").scaledToWidth(16, Qt::SmoothTransformation));
        ui->label_testConnectionStatus->setText("Connected");
    }
    else {
        ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_disconnected.png").scaledToWidth(16, Qt::SmoothTransformation));
        ui->label_testConnectionStatus->setText("Disconnected");
    }
}

void TorInfoDialog::onStatusChanged(const QString &msg) {
    ui->message->setText(msg);

    if (msg.isEmpty()) {
        ui->message->hide();
    }
}

TorInfoDialog::~TorInfoDialog() = default;