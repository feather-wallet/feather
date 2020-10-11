// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

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
    explicit TickerWidget(QWidget *parent, QString symbol, QString title = "", bool convertBalance = false);
    void removePctContainer();
    void setFiatText(QString &fiatCurrency, double amount, bool round);
    void setPctText(QString &text, bool positive);
    void setFontSizes();
    ~TickerWidget() override;


public slots:
    void init();

private:
    void setPercentHidden(bool hidden);

    Ui::TickerWidget *ui;
    QString m_symbol;
    bool m_convertBalance;
    AppContext *m_ctx;
};

#endif // TICKERWIDGET_H
