// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "URWidget.h"

#include "URWidget.h"
#include "ui_URWidget.h"

#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QTableWidget>

URWidget::URWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::URWidget)
{
    ui->setupUi(this);
    
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &URWidget::nextQR);
}

void URWidget::setData(const QString &type, const std::string &data) {
    std::string type_std = type.toStdString();

    ur::ByteVector a = ur::string_to_bytes(data);
    ur::ByteVector cbor;
    ur::CborLite::encodeBytes(cbor, a);
    ur::UR h = ur::UR(type_std, cbor);

    delete m_urencoder;
    m_urencoder = new ur::UREncoder(h, 100);
    
    allParts.clear();
    for (int i=0; i < m_urencoder->seq_len(); i++) {
        allParts.append(m_urencoder->next_part());
    }

    m_timer.start();
}

void URWidget::nextQR() {
    ui->label_seq->setText(QString("%1/%2").arg(QString::number(currentIndex % m_urencoder->seq_len() + 1), QString::number(m_urencoder->seq_len())));

    m_code = new QrCode{QString::fromStdString(allParts[currentIndex]), QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM};
    ui->qrWidget->setQrCode(m_code);
    
    currentIndex = (currentIndex + 1) % m_urencoder->seq_len();
}

URWidget::~URWidget() {
    delete m_urencoder;
}
