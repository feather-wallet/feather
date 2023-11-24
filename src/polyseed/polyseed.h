// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2021 tevador <tevador@gmail.com>

#include <polyseed.h>
#include <vector>
#include <stdexcept>
#include <string>

namespace polyseed {

    class data;

    class language {
    public:
        language() : m_lang(nullptr) {}
        language(const language&) = default;
        language(const polyseed_lang* lang) : m_lang(lang) {}
        const char* name() const {
            return polyseed_get_lang_name(m_lang);
        }
        const char* name_en() const {
            return polyseed_get_lang_name_en(m_lang);
        }
        bool valid() const {
            return m_lang != nullptr;
        }
    private:
        const polyseed_lang* m_lang;

        friend class data;
    };

    const std::vector<language>& get_langs();
    const language& get_lang_by_name(const std::string& name);

    class error : public std::runtime_error {
    public:
        error(const char* msg, polyseed_status status)
                : std::runtime_error(msg), m_status(status)
        {
        }
        polyseed_status status() const {
            return m_status;
        }
    private:
        polyseed_status m_status;
    };

    using feature_type = unsigned int;

    inline int enable_features(feature_type features) {
        return polyseed_enable_features(features);
    }

    class data {
    public:
        data(const data&) = delete;
        data(polyseed_coin coin) : m_data(nullptr), m_coin(coin) {}
        ~data() {
            polyseed_free(m_data);
        }

        void create(feature_type features);
        void create_from_secret(feature_type features, const char* secret);

        void load(polyseed_storage storage);

        language decode(const char* phrase);

        template<class str_type>
        void encode(const language& lang, str_type& str) const {
            check_valid();
            if (!lang.valid()) {
                throw std::runtime_error("invalid language");
            }
            str.resize(POLYSEED_STR_SIZE);
            auto size = polyseed_encode(m_data, lang.m_lang, m_coin, &str[0]);
            str.resize(size);
        }

        void save(polyseed_storage storage) const {
            check_valid();
            polyseed_store(m_data, storage);
        }

        void crypt(const char* password) {
            check_valid();
            polyseed_crypt(m_data, password);
        }

        void keygen(void* ptr, size_t key_size) const {
            check_valid();
            polyseed_keygen(m_data, m_coin, key_size, (uint8_t*)ptr);
        }

        bool valid() const {
            return m_data != nullptr;
        }

        bool encrypted() const {
            check_valid();
            return polyseed_is_encrypted(m_data);
        }

        uint64_t birthday() const {
            check_valid();
            return polyseed_get_birthday(m_data);
        }

        bool has_feature(feature_type feature) const {
            check_valid();
            return polyseed_get_feature(m_data, feature) != 0;
        }
    private:
        void check_valid() const {
            if (m_data == nullptr) {
                throw std::runtime_error("invalid object");
            }
        }
        void check_init() const;

        polyseed_data* m_data;
        polyseed_coin m_coin;
    };
}