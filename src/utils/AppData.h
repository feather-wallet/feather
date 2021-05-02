// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_APPDATA_H
#define FEATHER_APPDATA_H

#include <QObject>
#include <QPointer>
#include <QCoreApplication>

#include "prices.h"
#include "txfiathistory.h"
#include "RestoreHeightLookup.h"

class AppData : public QObject {
Q_OBJECT

public:
    explicit AppData(QObject *parent);
    static AppData* instance();

    Prices prices;
    TxFiatHistory *txFiatHistory;
    QMap<NetworkType::Type, int> heights;
    QMap<NetworkType::Type, RestoreHeightLookup*> restoreHeights;

private slots:
    void onBlockHeightsReceived(int mainnet, int stagenet);

private:
    void initRestoreHeights();

    static QPointer<AppData> m_instance;
};

inline AppData* appData()
{
    return AppData::instance();
}

#endif //FEATHER_APPDATA_H
