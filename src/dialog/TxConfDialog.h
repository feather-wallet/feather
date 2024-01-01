// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_TXCONFDIALOG_H
#define FEATHER_TXCONFDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/PendingTransaction.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TxConfDialog;
}

class TxConfDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit TxConfDialog(Wallet *wallet, PendingTransaction *tx, const QString &address, const QString &description, QWidget *parent = nullptr);
    ~TxConfDialog() override;

    bool showAdvanced = false;

private:
    void setShowAdvanced();

    QScopedPointer<Ui::TxConfDialog> ui;
    Wallet *m_wallet;
    PendingTransaction *m_tx;
    QString m_address;
    QString m_description;
};

#endif //FEATHER_TXCONFDIALOG_H
