// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "BroadcastTxDialog.h"
#include "ui_BroadcastTxDialog.h"
#include "utils/NetworkManager.h"

#include <QMessageBox>

BroadcastTxDialog::BroadcastTxDialog(QWidget *parent, QSharedPointer<AppContext> ctx, const QString &transactionHex)
        : QDialog(parent)
        , ui(new Ui::BroadcastTxDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    auto node = m_ctx->nodes->connection();
    m_rpc = new DaemonRpc(this, getNetworkTor(), node.toAddress());

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

    FeatherNode node = ui->radio_useCustom->isChecked() ? FeatherNode(ui->customNode->text()) : m_ctx->nodes->connection();

    if (node.isLocal()) {
        m_rpc->setNetwork(getNetworkClearnet());
    } else {
        m_rpc->setNetwork(getNetworkTor());
    }

    m_rpc->setDaemonAddress(node.toURL());
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

BroadcastTxDialog::~BroadcastTxDialog() = default;
