// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_QRCODEWIDGET_H
#define FEATHER_QRCODEWIDGET_H

#include <QWidget>

#include "qrcode/QrCode.h"

class QrCodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QrCodeWidget(QWidget *parent = nullptr);
    void setQrCode(QrCode *qrCode);

protected:
    void paintEvent(QPaintEvent *event) override;
    int heightForWidth(int w) const override;
    bool hasHeightForWidth() const override;

private:
    QrCode *m_qrcode = nullptr;
};

#endif //FEATHER_QRCODEWIDGET_H
