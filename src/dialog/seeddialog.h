// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_SEEDDIALOG_H
#define FEATHER_SEEDDIALOG_H

#include <QDialog>

namespace Ui {
    class SeedDialog;
}

class SeedDialog : public QDialog
{
Q_OBJECT

public:
    explicit SeedDialog(const QString& seed, QWidget *parent = nullptr);
    ~SeedDialog() override;

private:
    Ui::SeedDialog *ui;
};


#endif //FEATHER_SEEDDIALOG_H
