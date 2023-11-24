// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2021 tevador <tevador@gmail.com>

#include "polyseed.h"
#include "pbkdf2.h"

#include <sodium/core.h>
#include <sodium/utils.h>
#include <sodium/randombytes.h>
#include <boost/locale.hpp>

#include <array>

#include <QString>

#define POLYSEED_RANDBYTES 19

namespace polyseed {

    static std::locale locale;

    static size_t utf8_nfc(const char* str, polyseed_str norm) {
        auto Qstr = QString(str);
        auto Qs = Qstr.normalized(QString::NormalizationForm_C);
        auto s = Qs.toStdString();
        size_t size = std::min(s.size(), (size_t)POLYSEED_STR_SIZE - 1);
        s.copy(norm, size);
        norm[size] = '\0';
        sodium_memzero(&s[0], s.size());
        return size;
    }

    static size_t utf8_nfkd(const char* str, polyseed_str norm) {
        auto Qstr = QString(str);
        auto Qs = Qstr.normalized(QString::NormalizationForm_KD);
        auto s = Qs.toStdString();
        size_t size = std::min(s.size(), (size_t)POLYSEED_STR_SIZE - 1);
        s.copy(norm, size);
        norm[size] = '\0';
        sodium_memzero(&s[0], s.size());
        return size;
    }

    static char seed[POLYSEED_RANDBYTES]{};

    static void randombytes(void* const buf, const size_t size) {
        assert(size <= POLYSEED_RANDBYTES);
        if (std::all_of(seed, seed + size, [](char c) { return c == '\0'; })) {
            randombytes_buf(buf, size);
        } else {
            memcpy(buf, seed, size);
            sodium_memzero(seed, POLYSEED_RANDBYTES);
        }
    }

    struct dependency {
        dependency();
        std::vector<language> languages;
    };

    static dependency deps;

    dependency::dependency() {
        if (sodium_init() == -1) {
            throw std::runtime_error("sodium_init failed");
        }

        boost::locale::generator gen;
        gen.locale_cache_enabled(true);
        locale = gen("");

        sodium_memzero(seed, POLYSEED_RANDBYTES);

        polyseed_dependency pd;
        pd.randbytes = &randombytes;
        pd.pbkdf2_sha256 = &crypto_pbkdf2_sha256;
        pd.memzero = &sodium_memzero;
        pd.u8_nfc = &utf8_nfc;
        pd.u8_nfkd = &utf8_nfkd;
        pd.time = nullptr;
        pd.alloc = nullptr;
        pd.free = nullptr;

        polyseed_inject(&pd);

        for (int i = 0; i < polyseed_get_num_langs(); ++i) {
            languages.push_back(language(polyseed_get_lang(i)));
        }
    }

    static language invalid_lang;

    const std::vector<language>& get_langs() {
        return deps.languages;
    }

    const language& get_lang_by_name(const std::string& name) {
        for (auto& lang : deps.languages) {
            if (name == lang.name_en()) {
                return lang;
            }
            if (name == lang.name()) {
                return lang;
            }
        }
        return invalid_lang;
    }

    inline void data::check_init() const {
        if (valid()) {
            throw std::runtime_error("already initialized");
        }
    }

    static std::array<const char*, 7> error_desc = {
            "Success",
            "Wrong number of words in the phrase",
            "Unknown language or unsupported words",
            "Checksum mismatch",
            "Unsupported seed features",
            "Invalid seed format",
            "Memory allocation failure",
    };

    static error get_error(polyseed_status status) {
        if (status > 0 && status < sizeof(error_desc) / sizeof(const char*)) {
            return error(error_desc[(int)status], status);
        }
        return error("Unknown error", status);
    }

    void data::create(feature_type features) {
        check_init();
        auto status = polyseed_create(features, &m_data);
        if (status != POLYSEED_OK) {
            throw get_error(status);
        }
    }

    void data::create_from_secret(feature_type features, const char* secret) {
        check_init();
        memcpy(seed, secret, POLYSEED_RANDBYTES);
        auto status = polyseed_create(features, &m_data);
        if (status != POLYSEED_OK) {
            throw get_error(status);
        }
    }

    void data::load(polyseed_storage storage) {
        check_init();
        auto status = polyseed_load(storage, &m_data);
        if (status != POLYSEED_OK) {
            throw get_error(status);
        }
    }

    language data::decode(const char* phrase) {
        check_init();
        const polyseed_lang* lang;
        auto status = polyseed_decode(phrase, m_coin, &lang, &m_data);
        if (status != POLYSEED_OK) {
            throw get_error(status);
        }
        return language(lang);
    }
}