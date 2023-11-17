// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TXINFODIALOG_H
#define FEATHER_TXINFODIALOG_H

#include <QDialog>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QSvgWidget>

#include "dialog/TxProofDialog.h"
#include "libwalletqt/rows/TransactionRow.h"

namespace Ui {
    class TxInfoDialog;
}

class TxInfoDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxInfoDialog(Wallet *wallet, TransactionRow *txInfo, QWidget *parent = nullptr);
    ~TxInfoDialog() override;

signals:
    void resendTranscation(const QString &txid);

private:
    void copyTxID();
    void copyTxKey();
    void createTxProof();
    void setData(TransactionRow *tx);
    void updateData();
    void adjustHeight(QTextEdit *textEdit, qreal docHeight);
    void viewOnBlockExplorer();

    QScopedPointer<Ui::TxInfoDialog> ui;
    Wallet *m_wallet;
    TransactionRow *m_txInfo;
    TxProofDialog *m_txProofDialog;
    QString m_txid;
};

#endif //FEATHER_TXINFODIALOG_H
