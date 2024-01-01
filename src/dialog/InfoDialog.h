// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_INFODIALOG_H
#define FEATHER_INFODIALOG_H

#include <QDialog>

#include "components.h"

namespace Ui {
    class InfoDialog;
}

class InfoDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent, const QString &title, const QString &infoText);
    ~InfoDialog() override;

private:
    QScopedPointer<Ui::InfoDialog> ui;
};


#endif //FEATHER_INFODIALOG_H
