// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SIGNVERIFYDIALOG_H
#define FEATHER_SIGNVERIFYDIALOG_H

#include <QDialog>
#include "libwalletqt/Wallet.h"

namespace Ui {
    class SignVerifyDialog;
}

class SignVerifyDialog : public QDialog
{
Q_OBJECT

public:
    explicit SignVerifyDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~SignVerifyDialog() override;

private:
    Ui::SignVerifyDialog *ui;
    Wallet *m_wallet;

private slots:
    void signMessage();
    void verifyMessage();
    void copyToClipboard();
};


#endif //FEATHER_SIGNVERIFYDIALOG_H
