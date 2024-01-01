// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_URSETTINGSDIALOG_H
#define FEATHER_URSETTINGSDIALOG_H

#include <QDialog>

#include "components.h"

namespace Ui {
    class URSettingsDialog;
}

class URSettingsDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit URSettingsDialog(QWidget *parent = nullptr);
    ~URSettingsDialog() override;

private:
    QScopedPointer<Ui::URSettingsDialog> ui;
};


#endif //FEATHER_URSETTINGSDIALOG_H
