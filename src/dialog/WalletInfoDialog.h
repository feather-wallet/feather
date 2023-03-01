// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_WALLETINFODIALOG_H
#define FEATHER_WALLETINFODIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class WalletInfoDialog;
}

class WalletInfoDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit WalletInfoDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~WalletInfoDialog() override;

private:
    void openWalletDir();

    QScopedPointer<Ui::WalletInfoDialog> ui;
    Wallet *m_wallet;
};

#endif //FEATHER_WALLETINFODIALOG_H
