// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_TXPOOLVIEWERDIALOG_H
#define FEATHER_TXPOOLVIEWERDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TxPoolViewerDialog;
}

struct FeeTierStats {
  quint64 transactions = 0;
  quint64 totalWeight = 0;
  quint64 weightFromTip = 0;
};

class TxPoolViewerDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxPoolViewerDialog(QWidget *parent, Wallet *wallet);
    ~TxPoolViewerDialog() override;

private:
    void refresh();
    void onTxPoolBacklog(const QVector<TxBacklogEntry> &txPool, const QVector<quint64> &baseFees, quint64 blockWeightLimit);

    QVector<FeeTierStats> m_feeTierStats;
    QScopedPointer<Ui::TxPoolViewerDialog> ui;
    Wallet *m_wallet;
};


#endif //FEATHER_TXPOOLVIEWERDIALOG_H
