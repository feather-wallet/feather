// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_VERIFYPROOFDIALOG_H
#define FEATHER_VERIFYPROOFDIALOG_H

#include <QDialog>
#include <QIcon>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class VerifyProofDialog;
}

class VerifyProofDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit VerifyProofDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~VerifyProofDialog() override;

private slots:
    void checkProof();

private:
    void checkTxProof(const QString &txId, const QString &address, const QString &message, const QString &signature);
    void checkSpendProof(const QString &txId, const QString &message, const QString &signature);
    void checkOutProof();
    void checkInProof();
    void checkFormattedProof();
    void proofStatus(bool success, const QString &message);
    void onTxProofVerified(TxProofResult result);
    void onSpendProofVerified(QPair<bool, bool> result);

    QScopedPointer<Ui::VerifyProofDialog> ui;
    Wallet *m_wallet;

    QPixmap m_success;
    QPixmap m_failure;
};

#endif //FEATHER_VERIFYPROOFDIALOG_H
