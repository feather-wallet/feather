// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "QrCodeUtils.h"
#include <QDebug>

bool QrCodeUtils::zimageFromQImage(const QImage &qImg, zbar::Image &dst) {
    qDebug() << qImg.format();
    switch (qImg.format()) {
        case QImage::Format_RGB32 :
        case QImage::Format_ARGB32 :
        case QImage::Format_ARGB32_Premultiplied :
            break;
        default :
            return false;
    }

    unsigned int bpl(qImg.bytesPerLine());
    unsigned int width(bpl / 4);
    unsigned int height(qImg.height());

    dst.set_size(width, height);
    dst.set_format("BGR4");
    unsigned long datalen = qImg.sizeInBytes();
    dst.set_data(qImg.bits(), datalen);
    if ((width * 4 != bpl) || (width * height * 4 > datalen)) {
        return false;
    }
    return true;
}

QString QrCodeUtils::scanImage(const QImage &img) {
    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

    zbar::Image zImg;
    int r = zimageFromQImage(img, zImg);
    if (!r) {
        qWarning() << "Unable to convert QImage into zbar::Image";
        return "";
    }

    zbar::Image scanImg = zImg.convert(*(long*)"Y800");
    scanner.scan(scanImg);

    QString result;
    for (zbar::Image::SymbolIterator sym = scanImg.symbol_begin(); sym != scanImg.symbol_end(); ++sym) {
        if (!sym->get_count()) {
            result = QString::fromStdString(sym->get_data());
        }
    }
    return result;
}