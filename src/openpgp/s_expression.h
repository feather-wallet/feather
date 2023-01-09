// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#pragma once

#include <algorithm>
#include <stdexcept>

#include <gcrypt.h>

namespace openpgp
{

class s_expression
{
public:
  s_expression(const s_expression &) = delete;
  s_expression &operator=(const s_expression &) = delete;

  template <typename... Args>
  s_expression(Args... args)
  {
    if (gcry_sexp_build(&data, nullptr, args...) != GPG_ERR_NO_ERROR)
    {
      throw std::runtime_error("failed to build S-expression");
    }
  }

  s_expression(s_expression &&other)
  {
    std::swap(data, other.data);
  }

  s_expression(gcry_sexp_t data)
    : data(data)
  {
  }

  ~s_expression()
  {
    gcry_sexp_release(data);
  }

  const gcry_sexp_t &get() const
  {
    return data;
  }

private:
  gcry_sexp_t data = nullptr;
};

} // namespace openpgp
