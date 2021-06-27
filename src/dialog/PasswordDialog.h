// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_PASSWORDDIALOG_H
#define FEATHER_PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
    class PasswordDialog;
}

class PasswordDialog : public QDialog
{
Q_OBJECT

public:
    explicit PasswordDialog(const QString &walletName, bool incorrectPassword, QWidget *parent = nullptr);
    ~PasswordDialog() override;

    QString password = "";

private:
    Ui::PasswordDialog *ui;
};

#endif //FEATHER_PASSWORDDIALOG_H
