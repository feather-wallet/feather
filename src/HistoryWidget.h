// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_HISTORYWIDGET_H
#define FEATHER_HISTORYWIDGET_H

#include <QWidget>
#include <QMenu>

class TransactionHistoryProxyModel;
class Wallet;

namespace Ui {
    class HistoryWidget;
}

class HistoryWidget : public QWidget
{
Q_OBJECT

public:
    explicit HistoryWidget(Wallet *wallet, QWidget *parent = nullptr);
    ~HistoryWidget() override;

    void setSearchbarVisible(bool visible);
    void focusSearchbar();
    void setWebsocketEnabled(bool enabled);

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
    void onRemoveFromHistory();
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

    QScopedPointer<Ui::HistoryWidget> ui;
    Wallet *m_wallet;
    QMenu *m_contextMenu;
    QMenu *m_copyMenu;
    TransactionHistoryProxyModel *m_model;
};

#endif //FEATHER_HISTORYWIDGET_H
