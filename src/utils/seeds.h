// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_SEEDS_H
#define FEATHER_SEEDS_H

#include <QtCore>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <monero_seed/monero_seed.hpp>

#include "networktype.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "utils/utils.h"


struct RestoreHeightLookup {
    NetworkType::Type type;
    QMap<unsigned int, unsigned int> data;
    explicit RestoreHeightLookup(NetworkType::Type type) : type(type) {}

    unsigned int dateToRestoreHeight(unsigned int date) {
        // restore height based on a given timestamp using a lookup
        // table. If it cannot find the date in the lookup table, it
        // will calculate the blockheight based off the last known
        // date: ((now - lastKnownDate) / blockTime) - clearance

        if(this->type == NetworkType::TESTNET) return 1;
        unsigned int blockTime = 120;
        unsigned int blocksPerDay = 86400 / blockTime;
        unsigned int blockCalcClearance = blocksPerDay * 5;
        QList<unsigned int> values = this->data.keys();
        if(date <= values.at(0))
            return this->data[values.at(0)];
        for(unsigned int i = 0; i != values.count(); i++) {
            if(values[i] > date) {
                return i - 1 < 0 ? this->data[values[i]] : this->data[values[i-1]] - blockCalcClearance;
            }
        }

        // lookup failed, calculate blockheight from last known checkpoint
        unsigned int lastBlockHeightTime = values.at(values.count() - 1);
        unsigned int lastBlockHeight = this->data[lastBlockHeightTime];
        unsigned int deltaTime = date - lastBlockHeightTime;
        unsigned int deltaBlocks = deltaTime / blockTime;
        unsigned int blockHeight = (lastBlockHeight + deltaBlocks) - blockCalcClearance;
        qDebug() << "Calculated blockheight: " << blockHeight << " from epoch " << date;
        return blockHeight;
    }

    unsigned int restoreHeightToDate(unsigned int height) {
        // @TODO: most likely inefficient, refactor
        QMap<unsigned int, unsigned int>::iterator i;
        unsigned int timestamp = 0;
        for (i = this->data.begin(); i != this->data.end(); ++i) {
            unsigned int ts = i.key();
            if (i.value() > height)
                return timestamp;
            timestamp = ts;
        }
        return timestamp;
    }

    static RestoreHeightLookup *fromFile(const QString &fn, NetworkType::Type type) {
        // initialize this class using a lookup table, e.g `:/assets/restore_heights_monero_mainnet.txt`/
        auto rtn = new RestoreHeightLookup(type);
        auto data = Utils::barrayToString(Utils::fileOpen(fn));
        QMap<unsigned int, unsigned int> _data;
        for(const auto &line: data.split('\n')) {
            if(line.trimmed().isEmpty()) continue;
            auto spl = line.trimmed().split(':');
            rtn->data[spl.at(0).toUInt()] = spl.at(1).toUInt();
        }
        return rtn;
    }
};

struct FeatherSeed {
    QString mnemonicSeed;
    QString spendKey;
    time_t time = 0;
    unsigned int restoreHeight = 0;
    RestoreHeightLookup *lookup = nullptr;
    QString language = "English";
    std::string coinName;
    explicit FeatherSeed(RestoreHeightLookup *lookup, const std::string &coinName = "monero") : lookup(lookup), coinName(coinName) {}

    static FeatherSeed fromSeed(RestoreHeightLookup *lookup,
                                const std::string &coinName,
                                const std::string &mnemonicSeed) {

        auto rtn = FeatherSeed(lookup, coinName);
        rtn.coinName = coinName;
        rtn.lookup = lookup;
        rtn.mnemonicSeed = QString::fromStdString(mnemonicSeed);

        if(QString::fromStdString(mnemonicSeed).split(" ").count() == 14) {
            monero_seed seed(mnemonicSeed, coinName);
            std::stringstream buffer;
            buffer << seed.key();
            rtn.time = seed.date();
            rtn.setRestoreHeight();
            rtn.spendKey = QString::fromStdString(buffer.str());
        }
        return rtn;
    }

    static FeatherSeed generate(RestoreHeightLookup *lookup, const std::string &coinName) {
        auto rtn = FeatherSeed(lookup, coinName);
        time_t _time = std::time(nullptr);
        monero_seed seed(_time, coinName);

        std::stringstream buffer;
        buffer << seed;
        rtn.mnemonicSeed = QString::fromStdString(buffer.str());
        buffer.str(std::string());
        buffer << seed.key();
        rtn.spendKey = QString::fromStdString(buffer.str());
        rtn.time = _time;
        rtn.setRestoreHeight();
        return rtn;
    }

    Wallet *writeWallet(WalletManager *manager, NetworkType::Type type, const QString &path, const QString &password, unsigned int kdfRounds) {
        // writes both 14/25 word mnemonic seeds.
        Wallet *wallet = nullptr;
        if(this->lookup == nullptr) return wallet;
        if(this->mnemonicSeed.split(" ").count() == 14) {
            if(this->spendKey.isEmpty()) {
                auto _seed = FeatherSeed::fromSeed(this->lookup, this->coinName, this->mnemonicSeed.toStdString());
                _seed.setRestoreHeight();
                this->time = _seed.time;
                this->restoreHeight = _seed.restoreHeight;
                this->spendKey = _seed.spendKey;
            }
            wallet = manager->createDeterministicWalletFromSpendKey(path, password, this->language, type, this->spendKey, this->restoreHeight, (quint64)kdfRounds);
            wallet->setCacheAttribute("feather.seed", this->mnemonicSeed);
        } else {
            wallet = manager->recoveryWallet(path, password, this->mnemonicSeed, "", type, this->restoreHeight, (quint64) kdfRounds);
        }

        wallet->setPassword(password);
        return wallet;
    }

    unsigned int setRestoreHeight() {
        if(this->lookup == nullptr) return 1;
        if(this->time == 0) return 1;
        this->restoreHeight = this->lookup->dateToRestoreHeight((unsigned int)this->time);
        return this->restoreHeight;
    }

    unsigned int setRestoreHeight(unsigned int height) {
        auto now = (unsigned int)std::time(nullptr);
        auto nowClearance = 3600 * 24;
        auto currentBlockHeight = this->lookup->dateToRestoreHeight(now - nowClearance);
        if(height >= currentBlockHeight + nowClearance) {
            qCritical() << "unrealistic restore height detected, setting to current blockheight instead: " << currentBlockHeight;
            this->restoreHeight = currentBlockHeight;
        } else
            this->restoreHeight = height;

        return this->restoreHeight;
    }
};

#endif //FEATHER_SEEDS_H
