// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#pragma once

#include "mpi.h"

namespace openpgp
{

size_t bits_to_bytes(size_t bits)
{
  constexpr const uint16_t bits_in_byte = 8;
  return (bits + bits_in_byte - 1) / bits_in_byte;
}

std::string strip_line_breaks(const std::string &string)
{
  std::string result;
  result.reserve(string.size());
  for (const auto &character : string)
  {
    if (character != '\r' && character != '\n')
    {
      result.push_back(character);
    }
  }
  return result;
}

struct packet_tag
{
  enum type : uint8_t
  {
    signature = 2,
    public_key = 6,
    user_id = 13,
    public_subkey = 14,
  };

  const type packet_type;
  const size_t length;
};

template <
  typename byte_container,
  typename = typename std::enable_if<(sizeof(typename byte_container::value_type) == 1)>::type>
class deserializer
{
public:
  deserializer(byte_container buffer)
    : buffer(std::move(buffer))
    , cursor(0)
  {
  }

  bool empty() const
  {
    return buffer.size() - cursor == 0;
  }

  packet_tag read_packet_tag()
  {
    const auto tag = read_big_endian<uint8_t>();

    constexpr const uint8_t format_mask = 0b11000000;
    constexpr const uint8_t format_old_tag = 0b10000000;
    if ((tag & format_mask) != format_old_tag)
    {
      throw std::runtime_error("invalid packet tag");
    }

    const packet_tag::type packet_type = static_cast<packet_tag::type>((tag & 0b00111100) >> 2);
    const uint8_t length_type = tag & 0b00000011;

    size_t length;
    switch (length_type)
    {
    case 0:
      length = read_big_endian<uint8_t>();
      break;
    case 1:
      length = read_big_endian<uint16_t>();
      break;
    case 2:
      length = read_big_endian<uint32_t>();
      break;
    default:
      throw std::runtime_error("unsupported packet length type");
    }

    return {packet_type, length};
  }

  mpi read_mpi()
  {
    const size_t bit_length = read_big_endian<uint16_t>();
    return mpi(read_span(bits_to_bytes(bit_length)));
  }

  std::vector<uint8_t> read(size_t size)
  {
    if (buffer.size() - cursor < size)
    {
      throw std::runtime_error("insufficient buffer size");
    }

    const size_t offset = cursor;
    cursor += size;

    return {&buffer[offset], &buffer[cursor]};
  }

  template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
  T read_big_endian()
  {
    if (buffer.size() - cursor < sizeof(T))
    {
      throw std::runtime_error("insufficient buffer size");
    }
    T result = 0;
    for (size_t read = 0; read < sizeof(T); ++read)
    {
      result = (result << 8) | static_cast<uint8_t>(buffer[cursor++]);
    }
    return result;
  }

  epee::span<const uint8_t> read_span(size_t size)
  {
    if (buffer.size() - cursor < size)
    {
      throw std::runtime_error("insufficient buffer size");
    }

    const size_t offset = cursor;
    cursor += size;

    return {reinterpret_cast<const uint8_t *>(&buffer[offset]), size};
  }

private:
  byte_container buffer;
  size_t cursor;
};

} // namespace openpgp
