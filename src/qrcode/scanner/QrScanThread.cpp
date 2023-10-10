// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "QrScanThread.h"
#include <QDebug>

#include <ZXing/ReadBarcode.h>

QrScanThread::QrScanThread(QObject *parent)
    : QThread(parent)
    , m_running(true)
{
}

void QrScanThread::processQImage(const QImage &qimg)
{
    const auto hints = ZXing::DecodeHints()
            .setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::DataMatrix)
            .setTryHarder(true)
            .setBinarizer(ZXing::Binarizer::FixedThreshold);

    const auto result = QrCodeUtils::ReadBarcode(qimg, hints);

    if (result.isValid()) {
        emit decoded(result.text());
    }
}

void QrScanThread::stop()
{
    m_running = false;
    m_waitCondition.wakeOne();
}

void QrScanThread::addImage(const QImage &img)
{
    QMutexLocker locker(&m_mutex);
    m_queue.append(img);
    m_waitCondition.wakeOne();
}

void QrScanThread::run()
{
    while (m_running) {
        QMutexLocker locker(&m_mutex);
        while (m_queue.isEmpty() && m_running) {
            m_waitCondition.wait(&m_mutex);
        }
        if (!m_queue.isEmpty()) {
            processQImage(m_queue.takeFirst());
        }
    }
}