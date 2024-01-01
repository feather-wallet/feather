// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_TXDETAILSSIMPLE_H
#define FEATHER_TXDETAILSSIMPLE_H

#include <QWidget>

#include "components.h"
#include "libwalletqt/PendingTransaction.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TxDetailsSimple;
}

class TxDetailsSimple : public QWidget
{
    Q_OBJECT

public:
    explicit TxDetailsSimple(QWidget *parent);
    ~TxDetailsSimple() override;
    
    void setDetails(Wallet *wallet, PendingTransaction *tx, const QString &address);

private:
    QScopedPointer<Ui::TxDetailsSimple> ui;
};

#endif //FEATHER_TXDETAILSSIMPLE_H
