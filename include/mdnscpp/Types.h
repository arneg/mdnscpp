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

  static inline const char *describeIPProtocol(IPProtocol ipProtocol)
  {
    if (ipProtocol == IPProtocol::IPv4)
    {
      return "IPv4";
    }
    else if (ipProtocol == IPProtocol::IPv6)
    {
      return "IPv6";
    }
    else
    {
      return "Both";
    }
  }
} // namespace mdnscpp