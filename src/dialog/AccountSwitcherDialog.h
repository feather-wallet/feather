// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ACCOUNTSWITCHERDIALOG_H
#define FEATHER_ACCOUNTSWITCHERDIALOG_H

#include "components.h"
#include "libwalletqt/Wallet.h"
#include "model/SubaddressAccountModel.h"

namespace Ui {
    class AccountSwitcherDialog;
}

class AccountSwitcherDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit AccountSwitcherDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~AccountSwitcherDialog() override;

    void update();

private slots:
    void showContextMenu(const QPoint& point);
    void updateSelection();

private:
    void switchAccount();
    void copyLabel();
    void copyBalance();
    void editLabel();

    AccountRow* currentEntry();

    QScopedPointer<Ui::AccountSwitcherDialog> ui;
    Wallet *m_wallet;
    SubaddressAccountModel *m_model;
    SubaddressAccountProxyModel *m_proxyModel;
};

#endif //FEATHER_ACCOUNTSWITCHERDIALOG_H
