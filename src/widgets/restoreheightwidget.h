// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef RESTOREHEIGHTWIDGET_H
#define RESTOREHEIGHTWIDGET_H

#include <QWidget>
#include <QItemSelection>

#include "appcontext.h"

namespace Ui {
    class RestoreHeightWidget;
}

class RestoreHeightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RestoreHeightWidget(QWidget *parent = nullptr);
    void initRestoreHeights(RestoreHeightLookup *lookup);
    unsigned int getHeight();
    void hideSlider();
    ~RestoreHeightWidget();

private slots:
    void onValueChanged(int date);

private:
    RestoreHeightLookup *m_restoreHeightLookup = nullptr;
    Ui::RestoreHeightWidget *ui;
};

#endif // RESTOREHEIGHTWIDGET_H
