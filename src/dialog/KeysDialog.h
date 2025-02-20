// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_KEYSDIALOG_H
#define FEATHER_KEYSDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class KeysDialog;
}

class KeysDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit KeysDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~KeysDialog() override;

private:
    QScopedPointer<Ui::KeysDialog> ui;
};


#endif //FEATHER_KEYSDIALOG_H
