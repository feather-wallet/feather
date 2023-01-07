// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAYMENTREQUESTDIALOG_H
#define FEATHER_PAYMENTREQUESTDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"
#include "qrcode/QrCode.h"

namespace Ui {
    class PaymentRequestDialog;
}

class PaymentRequestDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit PaymentRequestDialog(QWidget *parent, QSharedPointer<AppContext> ctx, QString address);
    ~PaymentRequestDialog() override;

private slots:
    void updatePaymentRequest();
    void copyLink();
    void copyImage();
    void saveImage();

private:
    QScopedPointer<Ui::PaymentRequestDialog> ui;
    QSharedPointer<AppContext> m_ctx;
    QString m_address;
    QrCode *m_qrCode;
};

#endif //FEATHER_PAYMENTREQUESTDIALOG_H
