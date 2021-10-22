// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXIMPORTDIALOG_H
#define FEATHER_TXIMPORTDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"
#include "utils/daemonrpc.h"

namespace Ui {
    class TxImportDialog;
}

class TxImportDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit TxImportDialog(QWidget *parent, QSharedPointer<AppContext> ctx);
    ~TxImportDialog() override;

private slots:
    void loadTx();
    void onImport();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    QScopedPointer<Ui::TxImportDialog> ui;
    QSharedPointer<AppContext> m_ctx;

    DaemonRpc *m_rpc;
    QTimer *m_loadTimer;

    QJsonObject m_transaction;
};


#endif //FEATHER_TXIMPORTDIALOG_H
