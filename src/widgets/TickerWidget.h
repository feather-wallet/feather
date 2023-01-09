// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TICKERWIDGET_H
#define FEATHER_TICKERWIDGET_H

#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class TickerWidget;
}

class TickerWidgetBase : public QWidget
{
    Q_OBJECT

public:
    explicit TickerWidgetBase(QWidget *parent, QSharedPointer<AppContext> ctx);
    ~TickerWidgetBase() override;

    void setTitle(const QString &title);
    void setPercentageVisible(bool visible);

    void setPercentageText(const QString &text, bool positive);
    void setFiatText(double amount, const QString &fiatCurrency);
    void setDisplayText(const QString &text);

public slots:
    virtual void updateDisplay() = 0;

private:
    QScopedPointer<Ui::TickerWidget> ui;

protected:
    QSharedPointer<AppContext> m_ctx;
};

class BalanceTickerWidget : public TickerWidgetBase
{
    Q_OBJECT

public:
    explicit BalanceTickerWidget(QWidget *parent, QSharedPointer<AppContext> ctx, bool totalBalance);

public slots:
    void updateDisplay() override;

private:
    bool m_totalBalance;
};

class PriceTickerWidget : public TickerWidgetBase
{
    Q_OBJECT

public:
    explicit PriceTickerWidget(QWidget *parent, QSharedPointer<AppContext> ctx, QString symbol);

public slots:
    void updateDisplay() override;

private:
    QString m_symbol;
};

class RatioTickerWidget : public TickerWidgetBase
{
    Q_OBJECT

public:
    explicit RatioTickerWidget(QWidget *parent, QSharedPointer<AppContext> ctx, QString symbol1, QString symbol2);

public slots:
    void updateDisplay() override;

private:
    QString m_symbol1;
    QString m_symbol2;
};

#endif // FEATHER_TICKERWIDGET_H
