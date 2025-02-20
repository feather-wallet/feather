// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TAILSOS_H
#define TAILSOS_H

#include <QString>

class TailsOS
{
public:
    static bool detect();
    static bool detectDataPersistence();
    static bool detectDotPersistence();
    static QString version();

    static void showDataPersistenceDisabledWarning();
    static void persistXdgMime(const QString& filePath, const QString& data);

    static bool usePersistence;
    static bool rememberChoice;
    static const QString tailsPathData;

    static bool isTails;
    static bool detected;
};

#endif // TAILSOS_H
