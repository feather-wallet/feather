// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PASSWORDSETDIALOG_H
#define FEATHER_PASSWORDSETDIALOG_H

#include <QDialog>

#include "components.h"

namespace Ui {
    class PasswordSetDialog;
}

class PasswordSetDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit PasswordSetDialog(const QString &helpText, QWidget *parent = nullptr);
    ~PasswordSetDialog() override;

    QString password();

private:
    QScopedPointer<Ui::PasswordSetDialog> ui;
};


#endif //FEATHER_PASSWORDSETDIALOG_H
