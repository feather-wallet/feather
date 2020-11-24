// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef SENDWIDGET_H
#define SENDWIDGET_H

#include <QWidget>
#include "appcontext.h"
#include "widgets/ccswidget.h"

namespace Ui {
    class SendWidget;
}

class SendWidget : public QWidget
{
Q_OBJECT

public:
    explicit SendWidget(QWidget *parent = nullptr);
    void fill(const CCSEntry &entry);
    void fill(const QString &address, const QString& description);
    void fill(const QString &address, const QString& description, double amount);
    void fill(double amount);
    void clearFields();
    ~SendWidget() override;

public slots:
    void sendClicked();
    void clearClicked();
    void aliasClicked();
    void btnMaxClicked();
    void amountEdited(const QString &text);
    void addressEdited(const QString &text);
    void currencyComboChanged(int index);
    void fillAddress(const QString &address);
    void updateConversionLabel();
    void onOpenAliasResolveError(const QString &err);
    void onOpenAliasResolved(const QString &address, const QString &openAlias);
    void onWalletClosed();
    void onPreferredFiatCurrencyChanged();

    void onInitiateTransaction();
    void onEndTransaction();

signals:
    void resolveOpenAlias(const QString &address);
    void createTransaction(const QString &address, double amount, const QString &description, bool all);

private:
    void setupComboBox();

    Ui::SendWidget *ui;
    AppContext *m_ctx;
    double amount();
    double conversionAmount();
};

#endif // SENDWIDGET_H
