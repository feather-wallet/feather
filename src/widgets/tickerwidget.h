// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef TICKERWIDGET_H
#define TICKERWIDGET_H

#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class TickerWidget;
}

class TickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TickerWidget(QWidget *parent, AppContext *ctx, QString symbol, QString title = "", bool convertBalance = false, bool hidePercent = false);
    void setFiatText(QString &fiatCurrency, double amount);
    void setPctText(QString &text, bool positive);
    void setFontSizes();
    ~TickerWidget() override;


public slots:
    void init();

private:
    Ui::TickerWidget *ui;
    AppContext *m_ctx;
    QString m_symbol;
    bool m_convertBalance;
    bool m_hidePercent;
};

#endif // TICKERWIDGET_H
