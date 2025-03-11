// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_COINSWIDGET_H
#define FEATHER_COINSWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QSvgWidget>

#include "model/CoinsModel.h"
#include "model/CoinsProxyModel.h"
#include "libwalletqt/Coins.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class CoinsWidget;
}

class CoinsWidget : public QWidget
{
Q_OBJECT

public:
    explicit CoinsWidget(Wallet *wallet, QWidget *parent = nullptr);
    void setModel(CoinsModel * model, Coins * coins);
    ~CoinsWidget() override;

    void setSpendSelected(const QStringList &pubkeys);

signals:
    void spendSelectedChanged(const QStringList &pubkeys);

public slots:
    void setSearchbarVisible(bool visible);
    void focusSearchbar();

private slots:
    void showHeaderMenu(const QPoint& position);
    void setShowSpent(bool show);
    void freezeAllSelected();
    void thawAllSelected();
    void spendSelected();
    void viewOutput();
    void onSweepOutputs();
    void setSearchFilter(const QString &filter);
    void editLabel();

private:
    void freezeCoins(QStringList &pubkeys);
    void thawCoins(QStringList &pubkeys);
    void selectCoins(const QStringList &pubkeys);

    enum copyField {
        PubKey = 0,
        KeyImage,
        TxID,
        Address,
        Label,
        Height,
        Amount
    };

    QScopedPointer<Ui::CoinsWidget> ui;
    Wallet *m_wallet;

    QMenu *m_contextMenu;
    QMenu *m_headerMenu;
    QMenu *m_copyMenu;
    QAction *m_spendAction;
    QAction *m_showSpentAction;
    QAction *m_freezeOutputAction;
    QAction *m_freezeAllSelectedAction;
    QAction *m_thawOutputAction;
    QAction *m_thawAllSelectedAction;
    QAction *m_viewOutputAction;
    QAction *m_sweepOutputAction;
    QAction *m_sweepOutputsAction;
    QAction *m_editLabelAction;
    Coins *m_coins;
    CoinsModel * m_model;
    CoinsProxyModel * m_proxyModel;

    void showContextMenu(const QPoint & point);
    void copy(copyField field);
    QStringList selectedPubkeys();
    bool isCoinSpendable(const CoinsInfo& coin);
    QModelIndex getCurrentIndex();
};


#endif //FEATHER_COINSWIDGET_H
