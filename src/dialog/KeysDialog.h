// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_KEYSDIALOG_H
#define FEATHER_KEYSDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"

namespace Ui {
    class KeysDialog;
}

class KeysDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit KeysDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~KeysDialog() override;

private:
    QScopedPointer<Ui::KeysDialog> ui;
};


#endif //FEATHER_KEYSDIALOG_H
