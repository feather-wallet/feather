// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXBROADCASTDIALOG_H
#define FEATHER_TXBROADCASTDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "utils/daemonrpc.h"

namespace Ui {
    class TxBroadcastDialog;
}

class TxBroadcastDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxBroadcastDialog(QWidget *parent, QSharedPointer<AppContext> ctx, const QString &transactionHex = "");
    ~TxBroadcastDialog() override;

private slots:
    void broadcastTx();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    QScopedPointer<Ui::TxBroadcastDialog> ui;
    QSharedPointer<AppContext> m_ctx;
    UtilsNetworking *m_network;
    DaemonRpc *m_rpc;
};


#endif //FEATHER_TXBROADCASTDIALOG_H
