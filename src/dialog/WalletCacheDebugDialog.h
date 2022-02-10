// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_WALLETCACHEDEBUGDIALOG_H
#define FEATHER_WALLETCACHEDEBUGDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"

namespace Ui {
    class WalletCacheDebugDialog;
}

class WalletCacheDebugDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit WalletCacheDebugDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~WalletCacheDebugDialog() override;

private:
    void setOutput(const QString &output);

    QScopedPointer<Ui::WalletCacheDebugDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};



#endif //FEATHER_WALLETCACHEDEBUGDIALOG_H
