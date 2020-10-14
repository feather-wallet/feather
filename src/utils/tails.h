// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef TAILSOS_H
#define TAILSOS_H

#include <QApplication>


class TailsOS
{
public:
    static bool detect();
    static bool detectDataPersistence();
    static bool detectDotPersistence();
    static QString version();

    static void showDataPersistenceDisabledWarning();
    static void askPersistence();
    static void persistXdgMime(const QString& filePath, const QString& data);

    static bool usePersistence;
    static bool rememberChoice;
    static const QString tailsPathData;
};

#endif // TAILSOS_H
