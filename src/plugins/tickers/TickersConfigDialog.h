// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TICKERCONFIGDIALOG_H
#define TICKERCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "components.h"

namespace Ui {
    class TickersConfigDialog;
}

class TickersConfigDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit TickersConfigDialog(QWidget *parent = nullptr);
    ~TickersConfigDialog() override;

private:
    void addTicker();
    void removeTicker();
    void saveConfig();

    QScopedPointer<Ui::TickersConfigDialog> ui;
};

#endif //TICKERCONFIGDIALOG_H
