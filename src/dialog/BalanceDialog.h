// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_BALANCEDIALOG_H
#define FEATHER_BALANCEDIALOG_H

#include "libwalletqt/Wallet.h"

#include <QDialog>

namespace Ui {
    class BalanceDialog;
}

class BalanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BalanceDialog(QWidget *parent, Wallet *wallet);
    ~BalanceDialog() override;

private:
    QScopedPointer<Ui::BalanceDialog> ui;
};

#endif //FEATHER_BALANCEDIALOG_H
