// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef CalcWindow_H
#define CalcWindow_H

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

signals:
    void closed();

private:
    void closeEvent(QCloseEvent *bar) override;

private:
    Ui::CalcWindow *ui;
};

#endif // CalcWindow_H
