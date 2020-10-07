// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_VERIFYPROOFDIALOG_H
#define FEATHER_VERIFYPROOFDIALOG_H

#include <QDialog>
#include "libwalletqt/Wallet.h"

namespace Ui {
    class VerifyProofDialog;
}

class VerifyProofDialog : public QDialog
{
Q_OBJECT

public:
    explicit VerifyProofDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~VerifyProofDialog() override;

private slots:
    void checkSpendProof();
    void checkOutProof();
    void checkInProof();

private:
    void checkTxProof(const QString &txId, const QString &address, const QString &message, const QString &signature);

    Ui::VerifyProofDialog *ui;
    Wallet *m_wallet;
};

#endif //FEATHER_VERIFYPROOFDIALOG_H
