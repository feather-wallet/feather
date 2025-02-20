// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_OUTPUTINFODIALOG_H
#define FEATHER_OUTPUTINFODIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Coins.h"
#include "libwalletqt/rows/CoinsInfo.h"

namespace Ui {
    class OutputInfoDialog;
}

class OutputInfoDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit OutputInfoDialog(CoinsInfo *cInfo, QWidget *parent = nullptr);
    ~OutputInfoDialog() override;

private:
    QScopedPointer<Ui::OutputInfoDialog> ui;
};


#endif //FEATHER_OUTPUTINFODIALOG_H
