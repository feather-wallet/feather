// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_HISTORYWIDGET_H
#define FEATHER_HISTORYWIDGET_H

#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "libwalletqt/Coins.h"
#include "libwalletqt/Wallet.h"

#include <QWidget>
#include <QMenu>

namespace Ui {
    class HistoryWidget;
}

class HistoryWidget : public QWidget
{
Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    void setModel(TransactionHistoryProxyModel *model, Wallet *wallet);
    ~HistoryWidget() override;

public slots:
    void setSearchText(const QString &text);
    void resetModel();

signals:
    void viewOnBlockExplorer(QString txid);
    void resendTransaction(QString txid);

private slots:
    void showTxDetails();
    void onViewOnBlockExplorer();
    void setSearchFilter(const QString &filter);
    void onResendTransaction();

private:
    enum copyField {
        TxID = 0,
        Date,
        Amount
    };

    void copy(copyField field);
    void showContextMenu(const QPoint &point);

    Ui::HistoryWidget *ui;
    QMenu *m_contextMenu;
    QMenu *m_copyMenu;
    TransactionHistory *m_txHistory;
    TransactionHistoryProxyModel *m_model;
    Wallet *m_wallet = nullptr;
};

#endif //FEATHER_HISTORYWIDGET_H
