// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "QrCodeUtils.h"

Result QrCodeUtils::ReadBarcode(const QImage& img, const ZXing::DecodeHints& hints)
{
    auto ImgFmtFromQImg = [](const QImage& img){
        switch (img.format()) {
            case QImage::Format_ARGB32:
            case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
                return ZXing::ImageFormat::BGRX;

#else
                return ZXing::ImageFormat::XRGB;

#endif
            case QImage::Format_RGB888: return ZXing::ImageFormat::RGB;

            case QImage::Format_RGBX8888:
            case QImage::Format_RGBA8888: return ZXing::ImageFormat::RGBX;

            case QImage::Format_Grayscale8: return ZXing::ImageFormat::Lum;

            default: return ZXing::ImageFormat::None;
        }
    };

    auto exec = [&](const QImage& img){
        return Result(ZXing::ReadBarcode({ img.bits(), img.width(), img.height(), ImgFmtFromQImg(img) }, hints));
    };

    return ImgFmtFromQImg(img) == ZXing::ImageFormat::None ? exec(img.convertToFormat(QImage::Format_RGBX8888)) : exec(img);
}


QString QrCodeUtils::scanImage(const QImage &img) {
    const auto hints = ZXing::DecodeHints()
            .setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::DataMatrix)
            .setTryHarder(true)
            .setBinarizer(ZXing::Binarizer::FixedThreshold);

    const auto result = ReadBarcode(img, hints);

    return result.text();
}