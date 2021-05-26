// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_HISTORYWIDGET_H
#define FEATHER_HISTORYWIDGET_H

#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "libwalletqt/Coins.h"
#include "libwalletqt/Wallet.h"
#include "appcontext.h"

#include <QWidget>
#include <QMenu>

namespace Ui {
    class HistoryWidget;
}

class HistoryWidget : public QWidget
{
Q_OBJECT

public:
    explicit HistoryWidget(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~HistoryWidget() override;

    void setSearchbarVisible(bool visible);
    void focusSearchbar();

public slots:
    void setSearchText(const QString &text);
    void resetModel();
    void onWalletRefreshed();

signals:
    void viewOnBlockExplorer(QString txid);
    void resendTransaction(QString txid);

private slots:
    void showTxDetails();
    void onViewOnBlockExplorer();
    void setSearchFilter(const QString &filter);
    void onResendTransaction();
    void createTxProof();

private:
    enum copyField {
        TxID = 0,
        Description,
        Date,
        Amount
    };

    void copy(copyField field);
    void showContextMenu(const QPoint &point);
    void showSyncNoticeMsg();

    Ui::HistoryWidget *ui;
    QSharedPointer<AppContext> m_ctx;
    QMenu *m_contextMenu;
    QMenu *m_copyMenu;
    TransactionHistoryProxyModel *m_model;
};

#endif //FEATHER_HISTORYWIDGET_H
