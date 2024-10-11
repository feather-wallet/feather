// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_URDIALOG_H
#define FEATHER_URDIALOG_H

#include "components.h"

namespace Ui {
    class URDialog;
}

class URDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit URDialog(QWidget *parent);
    ~URDialog() override;

private:
    QScopedPointer<Ui::URDialog> ui;
};


#endif //FEATHER_URDIALOG_H
