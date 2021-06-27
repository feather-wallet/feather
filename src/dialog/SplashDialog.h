// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SPLASHDIALOG_H
#define FEATHER_SPLASHDIALOG_H

#include <QDialog>

namespace Ui {
    class SplashDialog;
}

class SplashDialog : public QDialog
{
Q_OBJECT

public:
    explicit SplashDialog(QWidget *parent = nullptr);
    ~SplashDialog() override;

    void setMessage(const QString &message);
    void setIcon(const QPixmap &icon);

private:
    Ui::SplashDialog *ui;
};

#endif //FEATHER_SPLASHDIALOG_H
