#ifndef FEATHER_FEATHERSEED_H
#define FEATHER_FEATHERSEED_H

#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "utils/AppData.h"

#include <sstream>
#include "RestoreHeightLookup.h"

enum SeedType {
    MONERO = 0, // 25 word seeds
    TEVADOR     // 14 word seeds
};

struct FeatherSeed {
    // TODO: this is spaghetti, needs refactor

    NetworkType::Type netType;

    QString coin;
    QString language;
    SeedType seedType;

    QStringList mnemonic;
    QString spendKey;
    QString correction;

    time_t time;
    int restoreHeight = 0;

    QString errorString;

    explicit FeatherSeed(NetworkType::Type networkType = NetworkType::MAINNET,
                          const QString &coin = "monero",
                          const QString &language = "English",
                          const QStringList &mnemonic = {})
            : netType(networkType), coin(coin), language(language), mnemonic(mnemonic)
    {
        // Generate a new mnemonic if none was given
        if (mnemonic.length() == 0) {
            this->time = std::time(nullptr);
            monero_seed seed(this->time, coin.toStdString());

            std::stringstream buffer;
            buffer << seed;
            this->mnemonic = QString::fromStdString(buffer.str()).split(" ");

            buffer.str(std::string());
            buffer << seed.key();
            this->spendKey = QString::fromStdString(buffer.str());

            this->setRestoreHeight();
        }

        if (mnemonic.length() == 25) {
            this->seedType = SeedType::MONERO;
        }
        else if (mnemonic.length() == 14) {
            this->seedType = SeedType::TEVADOR;
        } else {
            this->errorString = "Mnemonic seed does not match known type";
            return;
        }

        if (seedType == SeedType::TEVADOR) {
            try {
                monero_seed seed(mnemonic.join(" ").toStdString(), coin.toStdString());

                this->time = seed.date();
                this->setRestoreHeight();

                std::stringstream buffer;
                buffer << seed.key();
                this->spendKey = QString::fromStdString(buffer.str());

                this->correction = QString::fromStdString(seed.correction());
                if (!this->correction.isEmpty()) {
                    buffer.str(std::string());
                    buffer << seed;
                    int index = this->mnemonic.indexOf("xxxx");
                    this->mnemonic.replace(index, this->correction);
                }
            }
            catch (const std::exception &e) {
                this->errorString = e.what();
                return;
            }
        }
    }

    void setRestoreHeight() {
        if (this->time == 0)
            this->restoreHeight = 1;

        this->restoreHeight = appData()->restoreHeights[netType]->dateToRestoreHeight(this->time);
    }

    int setRestoreHeight(int height) {
        auto now = std::time(nullptr);
        auto nowClearance = 3600 * 24;
        auto currentBlockHeight = appData()->restoreHeights[netType]->dateToRestoreHeight(now - nowClearance);
        if (height >= currentBlockHeight + nowClearance) {
            qCritical() << "unrealistic restore height detected, setting to current blockheight instead: " << currentBlockHeight;
            this->restoreHeight = currentBlockHeight;
        } else {
            this->restoreHeight = height;
        }

        return this->restoreHeight;
    }
};

#endif //FEATHER_FEATHERSEED_H
