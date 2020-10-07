// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractButton>

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
    QStandardItemModel *m_model;
    Ui::AboutDialog *ui;
};

#endif // ABOUT_H
