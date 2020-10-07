// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef CALC_H
#define CALC_H

#include <QMainWindow>
#include "appcontext.h"

namespace Ui {
    class CalcWidget;
}

class CalcWidget : public QWidget
{
Q_OBJECT

public:
    explicit CalcWidget(QWidget *parent = nullptr);
    ~CalcWidget();

signals:
    void closed();

public slots:
    void fromChanged(const QString& data);
    void toChanged(const QString& data);
    void toComboChanged(const QString& data);
    void initFiat();
    void initCrypto();

private:
    Ui::CalcWidget *ui;
    AppContext *m_ctx = nullptr;
    bool m_comboBoxInit = false;
    void initComboBox();
    bool m_changing = false;
};

#endif // CALC_H
