// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TXIMPORTDIALOG_H
#define FEATHER_TXIMPORTDIALOG_H

#include <QDialog>

#include "components.h"
#include "utils/daemonrpc.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TxImportDialog;
}

class TxImportDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit TxImportDialog(QWidget *parent, Wallet *wallet);
    ~TxImportDialog() override;

private slots:
    void onImport();

private:
    QScopedPointer<Ui::TxImportDialog> ui;
    Wallet *m_wallet;
};


#endif //FEATHER_TXIMPORTDIALOG_H
