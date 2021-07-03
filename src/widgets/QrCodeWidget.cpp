// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "QrCodeWidget.h"

#include <QColor>
#include <QPainter>
#include <QPen>

QrCodeWidget::QrCodeWidget(QWidget *parent) : QWidget(parent)
{
}

void QrCodeWidget::setQrCode(QrCode *qrCode) {
    m_qrcode = qrCode;

    int k = m_qrcode->width();
    this->setMinimumSize(k*5, k*5);

    this->update();
}

void QrCodeWidget::paintEvent(QPaintEvent *event) {
    // Implementation adapted from Electrum: qrcodewidget.py
    if (!m_qrcode) {
        return;
    }

    QColor black{0, 0, 0, 255};
    QColor white{255, 255, 255, 255};
    QPen blackPen{black};
    blackPen.setJoinStyle(Qt::MiterJoin);

    QPainter painter(this);

    auto r = painter.viewport();
    int k = m_qrcode->width();
    int margin = 10;
    int framesize = std::min(r.width(), r.height());
    int boxsize = int((framesize - (2*margin)) / k);
    int size = k*boxsize;
    int left = (framesize - size)/2;
    int top = (framesize - size)/2;

    painter.setBrush(white);
    painter.setPen(white);
    painter.drawRect(0, 0, framesize, framesize);

    painter.setBrush(black);
    painter.setPen(blackPen);

    unsigned char* dot = m_qrcode->data();
    for (int row = 0; row < k; row++) {
        for (int column = 0; column < k; column++) {
            if (quint8(0x01) == (static_cast<quint8>(*dot++) & quint8(0x01))) {
                painter.drawRect(int(left+(column*boxsize)), int(top+(row*boxsize)), boxsize - 1, boxsize - 1);
            }
        }
    }
}