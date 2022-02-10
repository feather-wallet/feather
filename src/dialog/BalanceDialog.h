// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_BALANCEDIALOG_H
#define FEATHER_BALANCEDIALOG_H

#include <QDialog>

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
    QScopedPointer<Ui::BalanceDialog> ui;
};

#endif //FEATHER_BALANCEDIALOG_H
