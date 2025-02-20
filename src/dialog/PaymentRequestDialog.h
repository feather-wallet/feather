// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAYMENTREQUESTDIALOG_H
#define FEATHER_PAYMENTREQUESTDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"
#include "qrcode/QrCode.h"

namespace Ui {
    class PaymentRequestDialog;
}

class PaymentRequestDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit PaymentRequestDialog(QWidget *parent, Wallet *wallet, QString address);
    ~PaymentRequestDialog() override;

private slots:
    void updatePaymentRequest();
    void copyLink();
    void copyImage();
    void saveImage();

private:
    QScopedPointer<Ui::PaymentRequestDialog> ui;
    Wallet *m_wallet;
    QString m_address;
    QrCode *m_qrCode;
};

#endif //FEATHER_PAYMENTREQUESTDIALOG_H
