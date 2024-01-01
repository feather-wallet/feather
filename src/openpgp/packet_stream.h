// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#pragma once

#include <vector>

#include <span.h>

#include "serialization.h"

namespace openpgp
{

class packet_stream
{
public:
  packet_stream(const epee::span<const uint8_t> buffer)
    : packet_stream(deserializer<epee::span<const uint8_t>>(buffer))
  {
  }

  template <
    typename byte_container,
    typename = typename std::enable_if<(sizeof(typename byte_container::value_type) == 1)>::type>
  packet_stream(deserializer<byte_container> buffer)
  {
    while (!buffer.empty())
    {
      packet_tag tag = buffer.read_packet_tag();
      packets.push_back({std::move(tag), buffer.read(tag.length)});
    }
  }

  const std::vector<uint8_t> *find_first(packet_tag::type type) const
  {
    for (const auto &packet : packets)
    {
      if (packet.first.packet_type == type)
      {
        return &packet.second;
      }
    }
    return nullptr;
  }

  template <typename Callback>
  void for_each(packet_tag::type type, Callback &callback) const
  {
    for (const auto &packet : packets)
    {
      if (packet.first.packet_type == type)
      {
        callback(packet.second);
      }
    }
  }

private:
  std::vector<std::pair<packet_tag, std::vector<uint8_t>>> packets;
};

} // namespace openpgp
