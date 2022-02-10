// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_TXPROOFDIALOG_H
#define FEATHER_TXPROOFDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"
#include "libwalletqt/TransactionInfo.h"

namespace Ui {
    class TxProofDialog;
}

class TxProofDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit TxProofDialog(QWidget *parent, QSharedPointer<AppContext> ctx, TransactionInfo *txid);
    ~TxProofDialog() override;
    void setTxId(const QString &txid);
    void getTxKey();

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

    QScopedPointer<Ui::TxProofDialog> ui;
    QSharedPointer<AppContext> m_ctx;

    QStringList m_OutDestinations;
    QStringList m_InDestinations;
    QString m_txid;
    QString m_txKey;
    Mode m_mode;
    TransactionInfo::Direction m_direction;
};

#endif //FEATHER_TXPROOFDIALOG_H
