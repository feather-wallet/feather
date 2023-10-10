// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_QRCODEUTILS_H
#define FEATHER_QRCODEUTILS_H

#include <QImage>

#include <ZXing/ReadBarcode.h>

class Result : private ZXing::Result
{
public:
    explicit Result(ZXing::Result&& r) :
            m_result(std::move(r)){ }

    inline QString text() const { return QString::fromStdString(m_result.text()); }
    bool isValid() const { return m_result.isValid(); }

private:
    ZXing::Result m_result;
};

class QrCodeUtils {
public:
    static QString scanImage(const QImage &img);
    static Result ReadBarcode(const QImage& img, const ZXing::DecodeHints& hints = { });
};

#endif //FEATHER_QRCODEUTILS_H
