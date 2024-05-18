// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICCONFIGDIALOG_H
#define FEATHER_ATOMICCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "components.h"

namespace Ui {
    class AtomicConfigDialog;
}

class AtomicConfigDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit AtomicConfigDialog(QWidget *parent = nullptr);
    ~AtomicConfigDialog() override;

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

    QScopedPointer<Ui::AtomicConfigDialog> ui;
};


#endif //FEATHER_ATOMICCONFIGDIALOG_H
