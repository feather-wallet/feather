// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CALCWIDGET_H
#define FEATHER_CALCWIDGET_H

#include <QWidget>

namespace Ui {
    class CalcWidget;
}

class CalcWidget : public QWidget
{
Q_OBJECT

public:
    explicit CalcWidget(QWidget *parent = nullptr);
    ~CalcWidget() override;

public slots:
    void skinChanged();

private slots:
    void initComboBox();

private:
    void convert(bool reverse);

    Ui::CalcWidget *ui;
    bool m_comboBoxInit = false;
};

#endif // FEATHER_CALCWIDGET_H
