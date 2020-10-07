// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_MODELUTILS_H
#define FEATHER_MODELUTILS_H

#include <QString>
#include <QModelIndex>

class ModelUtils {
public:
    static QString displayAddress(const QString& address, int sections = 3, const QString & sep = " ");
    static void copyColumn(QModelIndex * index, int column);
    static QFont getMonospaceFont();
};


#endif //FEATHER_MODELUTILS_H
