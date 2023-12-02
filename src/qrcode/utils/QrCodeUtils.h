// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_QRCODEUTILS_H
#define FEATHER_QRCODEUTILS_H

#include <QImage>
#include <QString>

#include <ZXing/ReadBarcode.h>

class Result
{
public:
    explicit Result(const std::string &text, bool isValid)
        : m_text(QString::fromStdString(text))
        , m_valid(isValid){}
    
    [[nodiscard]] QString text() const { return m_text; }
    [[nodiscard]] bool isValid() const { return m_valid; }
    
private:
    QString m_text = "";
    bool m_valid = false;
};

class QrCodeUtils {
public:
    static QString scanImage(const QImage &img);
    static Result ReadBarcode(const QImage& img, const ZXing::DecodeHints& hints = { });
};

#endif //FEATHER_QRCODEUTILS_H
