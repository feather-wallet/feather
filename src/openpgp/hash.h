// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#pragma once

#include <vector>

#include <gcrypt.h>
#include <span.h>

namespace openpgp
{

class hash
{
public:
  enum algorithm : uint8_t
  {
    sha256 = 8,
  };

  hash(const hash &) = delete;
  hash &operator=(const hash &) = delete;

  hash(uint8_t algorithm)
    : algo(algorithm)
    , consumed(0)
  {
    if (gcry_md_open(&md, algo, 0) != GPG_ERR_NO_ERROR)
    {
      throw std::runtime_error("failed to create message digest object");
    }
  }

  ~hash()
  {
    gcry_md_close(md);
  }

  hash &operator<<(uint8_t byte)
  {
    gcry_md_putc(md, byte);
    ++consumed;
    return *this;
  }

  hash &operator<<(const epee::span<const uint8_t> &bytes)
  {
    gcry_md_write(md, &bytes[0], bytes.size());
    consumed += bytes.size();
    return *this;
  }

  hash &operator<<(const std::vector<uint8_t> &bytes)
  {
    return *this << epee::to_span(bytes);
  }

  std::vector<uint8_t> finish() const
  {
    std::vector<uint8_t> result(gcry_md_get_algo_dlen(algo));
    const void *digest = gcry_md_read(md, algo);
    if (digest == nullptr)
    {
      throw std::runtime_error("failed to read the digest");
    }
    memcpy(&result[0], digest, result.size());
    return result;
  }

  size_t consumed_bytes() const
  {
    return consumed;
  }

private:
  const uint8_t algo;
  gcry_md_hd_t md;
  size_t consumed;
};

}
