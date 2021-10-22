// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXINFODIALOG_H
#define FEATHER_TXINFODIALOG_H

#include <QDialog>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QtSvg/QSvgWidget>

#include "appcontext.h"
#include "dialog/TxProofDialog.h"

namespace Ui {
    class TxInfoDialog;
}

class TxInfoDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxInfoDialog(QSharedPointer<AppContext> ctx, TransactionInfo *txInfo, QWidget *parent = nullptr);
    ~TxInfoDialog() override;

signals:
    void resendTranscation(const QString &txid);

private:
    void copyTxID();
    void copyTxKey();
    void createTxProof();
    void setData(TransactionInfo *tx);
    void updateData();
    void adjustHeight(QTextEdit *textEdit, qreal docHeight);
    void viewOnBlockExplorer();

    QScopedPointer<Ui::TxInfoDialog> ui;
    QSharedPointer<AppContext> m_ctx;
    TransactionInfo *m_txInfo;
    TxProofDialog *m_txProofDialog;
    QString m_txid;
};

#endif //FEATHER_TXINFODIALOG_H
