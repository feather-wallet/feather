// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_CALCWINDOW_H
#define FEATHER_CALCWINDOW_H

#include <QMainWindow>

namespace Ui {
    class CalcWindow;
}

class CalcWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit CalcWindow(QWidget *parent = nullptr);
    ~CalcWindow() override;

private:
    QScopedPointer<Ui::CalcWindow> ui;
};

#endif // FEATHER_CALCWINDOW_H

