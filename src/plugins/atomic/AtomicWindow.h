// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICWINDOW_H
#define FEATHER_ATOMICWINDOW_H

#include <QMainWindow>

namespace Ui {
    class AtomicWindow;
}

class AtomicWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit AtomicWindow(QWidget *parent = nullptr);
    ~AtomicWindow() override;

private:
    QScopedPointer<Ui::AtomicWindow> ui;
};

#endif // FEATHER_ATOMICWINDOW_H

