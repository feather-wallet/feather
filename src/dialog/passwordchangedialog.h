// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_PASSWORDCHANGEDIALOG_H
#define FEATHER_PASSWORDCHANGEDIALOG_H

#include <QDialog>

namespace Ui {
    class PasswordChangeDialog;
}

class PasswordChangeDialog : public QDialog
{
Q_OBJECT

public:
    explicit PasswordChangeDialog(QWidget *parent = nullptr);
    ~PasswordChangeDialog() override;

    QString getCurrentPassword();
    QString getNewPassword();

private:
    Ui::PasswordChangeDialog *ui;

    void passwordsMatch();
};

#endif //FEATHER_PASSWORDCHANGEDIALOG_H
