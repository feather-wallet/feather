// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXPROOFDIALOG_H
#define FEATHER_TXPROOFDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "libwalletqt/TransactionInfo.h"

namespace Ui {
    class TxProofDialog;
}

class TxProofDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxProofDialog(QWidget *parent, QSharedPointer<AppContext> ctx, TransactionInfo *txid);
    ~TxProofDialog() override;
    void setTxId(const QString &txid);

private slots:
    void selectSpendProof();
    void selectOutProof();
    void selectInProof();
    void selectTxProof();

private:
    enum Mode {
        SpendProof = 0,
        OutProof,
        InProof
    };

    void getFormattedProof();
    void getSignature();
    TxProof getProof();
    void resetFrames();
    void toggleButtons(bool enabled);
    void showWarning(const QString &message);

    QStringList m_OutDestinations;
    QStringList m_InDestinations;
    QString m_txid;
    QString m_txKey;
    Mode m_mode;
    TransactionInfo::Direction m_direction;

    QScopedPointer<Ui::TxProofDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};

#endif //FEATHER_TXPROOFDIALOG_H
