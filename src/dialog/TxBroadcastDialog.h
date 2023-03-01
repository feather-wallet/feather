// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TXBROADCASTDIALOG_H
#define FEATHER_TXBROADCASTDIALOG_H

#include <QDialog>

#include "components.h"
#include "utils/daemonrpc.h"
#include "utils/nodes.h"

namespace Ui {
    class TxBroadcastDialog;
}

class TxBroadcastDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit TxBroadcastDialog(QWidget *parent, Nodes *nodes, const QString &transactionHex = "");
    ~TxBroadcastDialog() override;

private slots:
    void broadcastTx();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    QScopedPointer<Ui::TxBroadcastDialog> ui;
    Nodes *m_nodes;
    DaemonRpc *m_rpc;
};


#endif //FEATHER_TXBROADCASTDIALOG_H
