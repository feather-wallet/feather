// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SEEDS_H
#define FEATHER_SEEDS_H

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
    QMap<int, int> data;
    explicit RestoreHeightLookup(NetworkType::Type type) : type(type) {}

    int dateToRestoreHeight(int date) {
        // restore height based on a given timestamp using a lookup
        // table. If it cannot find the date in the lookup table, it
        // will calculate the blockheight based off the last known
        // date: ((now - lastKnownDate) / blockTime) - clearance

        if(this->type == NetworkType::TESTNET) return 1;
        int blockTime = 120;
        int blocksPerDay = 86400 / blockTime;
        int blockCalcClearance = blocksPerDay * 5;
        QList<int> values = this->data.keys();
        if(date <= values.at(0))
            return this->data[values.at(0)];
        for(int i = 0; i != values.count(); i++) {
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

    int restoreHeightToDate(int height) {
        // @TODO: most likely inefficient, refactor
        QMap<int, int>::iterator i;
        int timestamp = 0;
        for (i = this->data.begin(); i != this->data.end(); ++i) {
            int ts = i.key();
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
        QMap<int, int> _data;
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
    int restoreHeight = 0;
    RestoreHeightLookup *lookup = nullptr;
    QString language;
    std::string coinName;
    explicit FeatherSeed(RestoreHeightLookup *lookup, const std::string &coinName = "monero", const QString &language = "English") : lookup(lookup), coinName(coinName), language(language) {}

    static FeatherSeed fromSeed(RestoreHeightLookup *lookup,
                                const std::string &coinName,
                                const QString &seedLanguage,
                                const std::string &mnemonicSeed) {

        auto rtn = FeatherSeed(lookup, coinName, seedLanguage);
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

    static FeatherSeed generate(RestoreHeightLookup *lookup, const std::string &coinName, const QString &language) {
        auto rtn = FeatherSeed(lookup, coinName, language);
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

    Wallet *writeWallet(WalletManager *manager, NetworkType::Type type, const QString &path, const QString &password, quint64 kdfRounds) {
        // writes both 14/25 word mnemonic seeds.
        Wallet *wallet = nullptr;
        if(this->lookup == nullptr) return wallet;
        if(this->mnemonicSeed.split(" ").count() == 14) {
            if(this->spendKey.isEmpty()) {
                auto _seed = FeatherSeed::fromSeed(this->lookup, this->coinName, this->language, this->mnemonicSeed.toStdString());
                _seed.setRestoreHeight();
                this->time = _seed.time;
                this->restoreHeight = _seed.restoreHeight;
                this->spendKey = _seed.spendKey;
            }
            wallet = manager->createDeterministicWalletFromSpendKey(path, password, this->language, type, this->spendKey, this->restoreHeight, kdfRounds);
            wallet->setCacheAttribute("feather.seed", this->mnemonicSeed);
        } else {
            wallet = manager->recoveryWallet(path, password, this->mnemonicSeed, "", type, this->restoreHeight, kdfRounds);
        }

        wallet->setPassword(password);
        return wallet;
    }

    int setRestoreHeight() {
        if(this->lookup == nullptr) return 1;
        if(this->time == 0) return 1;
        this->restoreHeight = this->lookup->dateToRestoreHeight(this->time);
        return this->restoreHeight;
    }

    int setRestoreHeight(int height) {
        auto now = std::time(nullptr);
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
