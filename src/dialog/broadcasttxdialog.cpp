// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "broadcasttxdialog.h"
#include "ui_broadcasttxdialog.h"

#include <QMessageBox>

BroadcastTxDialog::BroadcastTxDialog(QWidget *parent, AppContext *ctx, const QString &transactionHex)
        : QDialog(parent)
        , m_ctx(ctx)
        , ui(new Ui::BroadcastTxDialog)
{
    ui->setupUi(this);

    m_network = new UtilsNetworking(m_ctx->network, this);

    auto node = ctx->nodes->connection();
    m_rpc = new DaemonRpc(this, m_network, node.full);

    connect(ui->btn_Broadcast, &QPushButton::clicked, this, &BroadcastTxDialog::broadcastTx);
    connect(ui->btn_Close, &QPushButton::clicked, this, &BroadcastTxDialog::reject);

    connect(m_rpc, &DaemonRpc::ApiResponse, this, &BroadcastTxDialog::onApiResponse);

    if (!transactionHex.isEmpty()) {
        ui->transaction->setPlainText(transactionHex);
    }

    this->adjustSize();
}

void BroadcastTxDialog::broadcastTx() {
    QString tx = ui->transaction->toPlainText();

    QString node = [this]{
        QString node;
        if (ui->radio_useCustom->isChecked())
            node = ui->customNode->text();
        else if (ui->radio_useDefault->isChecked())
            node = m_ctx->nodes->connection().full;

        if (!node.startsWith("http://"))
            node = QString("http://%1").arg(node);
        return node;
    }();

    m_rpc->setDaemonAddress(node);
    m_rpc->sendRawTransaction(tx);
}

void BroadcastTxDialog::onApiResponse(const DaemonRpc::DaemonResponse &resp) {
    if (!resp.ok) {
        QMessageBox::warning(this, "Transaction broadcast", resp.status);
        return;
    }

    if (resp.endpoint == DaemonRpc::Endpoint::SEND_RAW_TRANSACTION) {
        QMessageBox::information(this, "Transaction broadcast", "Transaction submitted successfully.\n\n"
                                                      "If the transaction belongs to this wallet it may take several minutes before it shows up in the history tab.");
    }
}

BroadcastTxDialog::~BroadcastTxDialog() {
    delete ui;
}
