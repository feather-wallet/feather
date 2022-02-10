// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_WALLETINFODIALOG_H
#define FEATHER_WALLETINFODIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"

namespace Ui {
    class WalletInfoDialog;
}

class WalletInfoDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit WalletInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~WalletInfoDialog() override;

private:
    void openWalletDir();

    QScopedPointer<Ui::WalletInfoDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};

#endif //FEATHER_WALLETINFODIALOG_H
