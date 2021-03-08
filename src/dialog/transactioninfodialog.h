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
#include "dialog/TxProofDialog.h"

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
    void createTxProof();

    Ui::TransactionInfoDialog *ui;

    TxProofDialog *m_txProofDialog;
    TransactionInfo *m_txInfo;
    Wallet *m_wallet;
    QString m_txKey;
    QString m_txid;
};

#endif //FEATHER_TRANSACTIONINFODIALOG_H
