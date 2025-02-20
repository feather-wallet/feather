// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_ABOUT_H
#define FEATHER_ABOUT_H

#include "components.h"

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private:
    QScopedPointer<Ui::AboutDialog> ui;
};

#endif // FEATHER_ABOUT_H
