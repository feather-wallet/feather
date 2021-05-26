// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CALCCONFIGDIALOG_H
#define FEATHER_CALCCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
    class CalcConfigDialog;
}

class CalcConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalcConfigDialog(QWidget *parent = nullptr);
    ~CalcConfigDialog() override;

    QStringList checkedFiat();
    QStringList checkedCrypto();

private slots:
    void selectAll();
    void deselectAll();

private:
    void setCheckState(QListWidget *widget, Qt::CheckState checkState);
    QStringList getChecked(QListWidget *widget);
    void fillListWidgets();
    QListWidget* getVisibleListWidget();

    Ui::CalcConfigDialog *ui;
};


#endif //FEATHER_CALCCONFIGDIALOG_H
