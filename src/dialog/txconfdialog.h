// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_TXCONFDIALOG_H
#define FEATHER_TXCONFDIALOG_H

#include <QDialog>
#include "libwalletqt/PendingTransaction.h"
#include "libwalletqt/WalletManager.h"

namespace Ui {
    class TxConfDialog;
}

class TxConfDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxConfDialog(PendingTransaction *tx, const QString &address, const QString &description, int mixin, QWidget *parent = nullptr);
    ~TxConfDialog() override;

private:
    void showAdvanced();

    Ui::TxConfDialog *ui;
    PendingTransaction *m_tx;
    QString m_address;
    QString m_description;
    int m_mixin;
};

#endif //FEATHER_TXCONFDIALOG_H
