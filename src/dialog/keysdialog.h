// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_KEYSDIALOG_H
#define FEATHER_KEYSDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class KeysDialog;
}

class KeysDialog : public QDialog
{
Q_OBJECT

public:
    explicit KeysDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~KeysDialog() override;

private:
    Ui::KeysDialog *ui;
};


#endif //FEATHER_KEYSDIALOG_H
