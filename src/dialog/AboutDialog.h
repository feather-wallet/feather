// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QStringListModel>
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
    Ui::AboutDialog *ui;
    QStringListModel *m_model;
};

#endif // ABOUT_H
