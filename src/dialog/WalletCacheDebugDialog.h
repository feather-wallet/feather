// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WALLETCACHEDEBUGDIALOG_H
#define FEATHER_WALLETCACHEDEBUGDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class WalletCacheDebugDialog;
}

class WalletCacheDebugDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit WalletCacheDebugDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~WalletCacheDebugDialog() override;

private:
    void setOutput(const QString &output);

    QScopedPointer<Ui::WalletCacheDebugDialog> ui;
    Wallet *m_wallet;
};



#endif //FEATHER_WALLETCACHEDEBUGDIALOG_H
