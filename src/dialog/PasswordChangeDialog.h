// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_PASSWORDCHANGEDIALOG_H
#define FEATHER_PASSWORDCHANGEDIALOG_H

#include <QDialog>
#include "libwalletqt/Wallet.h"

namespace Ui {
    class PasswordChangeDialog;
}

class PasswordChangeDialog : public QDialog
{
Q_OBJECT

public:
    explicit PasswordChangeDialog(QWidget *parent, Wallet *wallet);
    ~PasswordChangeDialog() override;

private:
    QScopedPointer<Ui::PasswordChangeDialog> ui;
    Wallet *m_wallet;

    void passwordsMatch();
    void setPassword();
};

#endif //FEATHER_PASSWORDCHANGEDIALOG_H
