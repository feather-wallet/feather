// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WALLETCACHEDEBUGDIALOG_H
#define FEATHER_WALLETCACHEDEBUGDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class WalletCacheDebugDialog;
}

class WalletCacheDebugDialog : public QDialog
{
Q_OBJECT

public:
    explicit WalletCacheDebugDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~WalletCacheDebugDialog() override;

private:
    void setOutput(const QString &output);
    Ui::WalletCacheDebugDialog *ui;
    AppContext *m_ctx;
};



#endif //FEATHER_WALLETCACHEDEBUGDIALOG_H
