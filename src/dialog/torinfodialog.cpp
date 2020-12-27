// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "torinfodialog.h"
#include "ui_torinfodialog.h"

#include <QDesktopServices>

TorInfoDialog::TorInfoDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TorInfoDialog)
        , m_ctx(ctx)
{
    ui->setupUi(this);

    if (!m_ctx->tor->torConnected && !m_ctx->tor->errorMsg.isEmpty()) {
        ui->message->setText(m_ctx->tor->errorMsg);
    } else {
        ui->message->setText(QString("Currently using Tor instance: %1:%2").arg(Tor::torHost).arg(Tor::torPort));
    }

    if (m_ctx->tor->localTor) {
        ui->logs->setHidden(true);
    } else {
        ui->logs->setPlainText(m_ctx->tor->torLogs);
    }

    this->adjustSize();
}

void TorInfoDialog::onLogsUpdated() {
    ui->logs->setPlainText(m_ctx->tor->torLogs);
}

TorInfoDialog::~TorInfoDialog() {
    delete ui;
}
