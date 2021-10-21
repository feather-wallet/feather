// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_ABOUT_H
#define FEATHER_ABOUT_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private:
    QScopedPointer<Ui::AboutDialog> ui;
};

#endif // FEATHER_ABOUT_H
