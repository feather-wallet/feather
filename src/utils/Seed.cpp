// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include <iomanip>
#include "Seed.h"

Seed::Seed(Type type, NetworkType::Type networkType, QString language, const char* secret)
    : type(type)
    , networkType(networkType)
    , language(std::move(language))
{
    // We only support the creation of Polyseeds
    if (this->type != Type::POLYSEED) {
        this->errorString = "Unsupported seed type";
        return;
    }

    // We only support the creation of English language seeds for now.
    if (this->language != "English") {
        this->errorString = "Unsupported seed language";
        return;
    }

    this->time = std::time(nullptr);

    try {
        polyseed::data seed(POLYSEED_MONERO);

        if (secret) {
            seed.create_from_secret(0, secret);
        } else {
            seed.create(0);
        }

        uint8_t key[32];
        seed.keygen(&key, sizeof(key));

        std::stringstream keyStream;
        for (unsigned char i : key) {
            keyStream << std::hex << std::setfill('0') << std::setw(2) << (int)i;
        }

        std::string phrase;
        seed.encode(polyseed::get_lang_by_name("English"), phrase);

        this->mnemonic = QString::fromStdString(phrase).split(" ");
        this->spendKey = QString::fromStdString(keyStream.str());
    }
    catch (const std::exception &e) {
        this->errorString = QString::fromStdString(e.what());
        return;
    }

    // Basic check against seed library implementation issues
    if (m_insecureSeeds.contains(this->spendKey)) {
        this->errorString = "Insecure spendkey";
        return;
    }

    this->setRestoreHeight();
}

Seed::Seed(Type type, QStringList mnemonic, NetworkType::Type networkType)
    : type(type), mnemonic(std::move(mnemonic)), networkType(networkType)
{
    if (m_seedLength[this->type] != this->mnemonic.length()) {
        this->errorString = "Invalid seed length";
        return;
    }

    if (this->type == Type::POLYSEED) {
        try {
            polyseed::data seed(POLYSEED_MONERO);
            auto lang = seed.decode(this->mnemonic.join(" ").toStdString().c_str());

            uint8_t key[32];
            seed.keygen(&key, sizeof(key));

            std::stringstream keyStream;
            for (unsigned char i : key) {
                keyStream << std::hex << std::setfill('0') << std::setw(2) << (int)i;
            }
            this->spendKey = QString::fromStdString(keyStream.str());

            this->time = seed.birthday();
            this->setRestoreHeight();

            this->encrypted = seed.encrypted();
        }
        catch (const std::exception &e) {
            this->errorString = e.what();
            return;
        }
    }

    if (this->type == Type::TEVADOR) {
        try {
            monero_seed seed(this->mnemonic.join(" ").toStdString(), constants::coinName);

            this->time = seed.date();
            this->setRestoreHeight();

            std::stringstream buffer;
            buffer << seed.key();
            this->spendKey = QString::fromStdString(buffer.str());

            // Tevador style seeds have built-in error correction
            // Any word can be replaced with 'xxxx' to recover the original word
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

void Seed::setRestoreHeight(int height) {
    auto now = std::time(nullptr);
    auto nowClearance = 3600 * 24;
    auto currentBlockHeight = appData()->restoreHeights[this->networkType]->dateToHeight(now - nowClearance);
    if (height >= currentBlockHeight + nowClearance) {
        qWarning() << "unrealistic restore height detected, setting to current blockheight instead: " << currentBlockHeight;
        this->restoreHeight = currentBlockHeight;
    } else {
        this->restoreHeight = height;
    }
}

void Seed::setRestoreHeight() {
    // Ignore the embedded restore date, new wallets should sync from the current block height.
    this->restoreHeight = appData()->restoreHeights[networkType]->dateToHeight(this->time);
}

Seed::Seed() = default;