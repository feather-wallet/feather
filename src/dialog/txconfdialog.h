// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXCONFDIALOG_H
#define FEATHER_TXCONFDIALOG_H

#include <QDialog>
#include "libwalletqt/PendingTransaction.h"
#include "libwalletqt/WalletManager.h"
#include "appcontext.h"

namespace Ui {
    class TxConfDialog;
}

class TxConfDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxConfDialog(AppContext *ctx, PendingTransaction *tx, const QString &address, const QString &description, int mixin, QWidget *parent = nullptr);
    ~TxConfDialog() override;

    bool showAdvanced = false;

private:
    void setShowAdvanced();
    void saveToFile();
    void copyToClipboard();

    Ui::TxConfDialog *ui;
    AppContext *m_ctx;
    PendingTransaction *m_tx;
    QString m_address;
    QString m_description;
    int m_mixin;
};

#endif //FEATHER_TXCONFDIALOG_H
