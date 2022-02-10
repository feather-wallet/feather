// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_QRCODEUTILS_H
#define FEATHER_QRCODEUTILS_H

#include <QImage>
#include <zbar.h>

class QrCodeUtils {
public:
    static bool zimageFromQImage(const QImage &qImg, zbar::Image &dst);
    static QString scanImage(const QImage &img);
};


#endif //FEATHER_QRCODEUTILS_H
