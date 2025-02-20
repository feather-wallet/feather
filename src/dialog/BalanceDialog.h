// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_BALANCEDIALOG_H
#define FEATHER_BALANCEDIALOG_H

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class BalanceDialog;
}

class BalanceDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit BalanceDialog(QWidget *parent, Wallet *wallet);
    ~BalanceDialog() override;

private:
    void updateBalance();

    QScopedPointer<Ui::BalanceDialog> ui;
    Wallet *m_wallet;
};

#endif //FEATHER_BALANCEDIALOG_H
