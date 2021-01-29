// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TRANSACTIONINFODIALOG_H
#define FEATHER_TRANSACTIONINFODIALOG_H

#include <QDialog>
#include <QTextCharFormat>
#include <QtSvg/QSvgWidget>
#include "libwalletqt/Coins.h"
#include "libwalletqt/TransactionInfo.h"
#include "libwalletqt/Wallet.h"
#include "widgets/txproofwidget.h"

namespace Ui {
    class TransactionInfoDialog;
}

class TransactionInfoDialog : public QDialog
{
Q_OBJECT

public:
    explicit TransactionInfoDialog(Wallet *wallet, TransactionInfo *txInfo, QWidget *parent = nullptr);
    ~TransactionInfoDialog() override;

private:
    void copyTxKey();

    Ui::TransactionInfoDialog *ui;

    TransactionInfo *m_txInfo;
    Wallet *m_wallet;
    TxProofWidget *m_txProofWidget;
    QString m_txKey;
};

#endif //FEATHER_TRANSACTIONINFODIALOG_H
