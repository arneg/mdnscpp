#pragma once

#include <cstdint>

namespace mdnscpp
{
  enum class IPProtocol : uint8_t
  {
    IPv4 = 1,
    IPv6 = 2,
    Both = 3,
  };
}