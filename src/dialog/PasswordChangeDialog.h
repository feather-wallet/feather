// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PASSWORDCHANGEDIALOG_H
#define FEATHER_PASSWORDCHANGEDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class PasswordChangeDialog;
}

class PasswordChangeDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit PasswordChangeDialog(QWidget *parent, Wallet *wallet);
    ~PasswordChangeDialog() override;

private:
    void passwordsMatch();
    void setPassword();

    QScopedPointer<Ui::PasswordChangeDialog> ui;
    Wallet *m_wallet;
};

#endif //FEATHER_PASSWORDCHANGEDIALOG_H
