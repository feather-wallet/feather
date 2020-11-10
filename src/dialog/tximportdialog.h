#ifndef FEATHER_TXIMPORTDIALOG_H
#define FEATHER_TXIMPORTDIALOG_H

#include <QDialog>
#include "appcontext.h"
#include "utils/daemonrpc.h"

namespace Ui {
    class TxImportDialog;
}

class TxImportDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxImportDialog(QWidget *parent, AppContext *ctx);
    ~TxImportDialog() override;

private slots:
    void loadTx();
    void onImport();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    Ui::TxImportDialog *ui;
    UtilsNetworking *m_network;
    AppContext *m_ctx;
    DaemonRpc *m_rpc;
    QTimer *m_loadTimer;

    QJsonObject m_transaction;
};


#endif //FEATHER_TXIMPORTDIALOG_H
