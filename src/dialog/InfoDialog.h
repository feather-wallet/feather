// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_INFODIALOG_H
#define FEATHER_INFODIALOG_H

#include <QDialog>

namespace Ui {
    class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent, const QString &title, const QString &infoText);
    ~InfoDialog() override;

private:
    Ui::InfoDialog *ui;
};


#endif //FEATHER_INFODIALOG_H
