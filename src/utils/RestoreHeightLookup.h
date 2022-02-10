// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_RESTOREHEIGHTLOOKUP_H
#define FEATHER_RESTOREHEIGHTLOOKUP_H

#include <cstdio>
#include <cstdlib>

#include <monero_seed/monero_seed.hpp>

#include "networktype.h"
#include "utils/Utils.h"

struct RestoreHeightLookup {
    NetworkType::Type type;
    QMap<int, int> data;
    explicit RestoreHeightLookup(NetworkType::Type type) : type(type) {}

    int dateToHeight(int date) {
        // restore height based on a given timestamp using a lookup
        // table. If it cannot find the date in the lookup table, it
        // will calculate the blockheight based off the last known
        // date: ((now - lastKnownDate) / blockTime) - clearance

        if (this->type == NetworkType::TESTNET) {
            return 1;
        }

        int blockTime = 120;
        int blocksPerDay = 86400 / blockTime;
        int blockCalcClearance = blocksPerDay * 5;
        QList<int> values = this->data.keys();
        if (date <= values.at(0))
            return this->data[values.at(0)];
        for (int i = 0; i != values.count(); i++) {
            if(values[i] > date) {
                return i - 1 < 0 ? this->data[values[i]] : this->data[values[i-1]] - blockCalcClearance;
            }
        }

        // lookup failed, calculate blockheight from last known checkpoint
        int lastBlockHeightTime = values.at(values.count() - 1);
        int lastBlockHeight = this->data[lastBlockHeightTime];
        int deltaTime = date - lastBlockHeightTime;
        int deltaBlocks = deltaTime / blockTime;
        int blockHeight = (lastBlockHeight + deltaBlocks) - blockCalcClearance;
        qDebug() << "Calculated blockheight: " << blockHeight << " from epoch " << date;
        return blockHeight;
    }

    int heightToTimestamp(int height) {
        // @TODO: most likely inefficient, refactor
        QMap<int, int>::iterator i;
        int timestamp = 0;
        int heightData = 1;
        for (i = this->data.begin(); i != this->data.end(); ++i) {
            int ts = i.key();
            if (i.value() > height)
                return timestamp;
            timestamp = ts;
            heightData = i.value();
        }

        while (heightData < height) {
            heightData += 720; // blocks per day
            timestamp += 86400; // seconds in day
        }

        return timestamp;
    }

    QDateTime heightToDate(int height) {
        return QDateTime::fromSecsSinceEpoch(this->heightToTimestamp(height));
    }

    static RestoreHeightLookup *fromFile(const QString &fn, NetworkType::Type type) {
        // initialize this class using a lookup table, e.g `:/assets/restore_heights_monero_mainnet.txt`/
        auto rtn = new RestoreHeightLookup(type);
        auto data = Utils::barrayToString(Utils::fileOpen(fn));
        QMap<int, int> _data;
        for(const auto &line: data.split('\n')) {
            if(line.trimmed().isEmpty()) continue;
            auto spl = line.trimmed().split(':');
            rtn->data[spl.at(0).toInt()] = spl.at(1).toInt();
        }
        return rtn;
    }
};

#endif //FEATHER_RESTOREHEIGHTLOOKUP_H
