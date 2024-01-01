// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#pragma once

#include <gcrypt.h>

namespace openpgp
{

class mpi
{
public:
  mpi(const mpi &) = delete;
  mpi &operator=(const mpi &) = delete;

  mpi(mpi &&other)
    : data(other.data)
  {
    other.data = nullptr;
  }

  template <
    typename byte_container,
    typename = typename std::enable_if<(sizeof(typename byte_container::value_type) == 1)>::type>
  mpi(const byte_container &buffer, gcry_mpi_format format = GCRYMPI_FMT_USG)
    : mpi(&buffer[0], buffer.size(), format)
  {
  }

  mpi(const void *buffer, size_t size, gcry_mpi_format format = GCRYMPI_FMT_USG)
  {
    if (gcry_mpi_scan(&data, format, buffer, size, nullptr) != GPG_ERR_NO_ERROR)
    {
      throw std::runtime_error("failed to read mpi from buffer");
    }
  }

  ~mpi()
  {
    gcry_mpi_release(data);
  }

  const gcry_mpi_t &get() const
  {
    return data;
  }

private:
  gcry_mpi_t data;
};

} // namespace openpgp
