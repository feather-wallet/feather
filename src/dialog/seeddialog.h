// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SEEDDIALOG_H
#define FEATHER_SEEDDIALOG_H

#include <QDialog>
#include "libwalletqt/Wallet.h"

namespace Ui {
    class SeedDialog;
}

class SeedDialog : public QDialog
{
Q_OBJECT

public:
    explicit SeedDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~SeedDialog() override;

private:
    void setSeed(const QString &seed);

    Ui::SeedDialog *ui;
};


#endif //FEATHER_SEEDDIALOG_H
