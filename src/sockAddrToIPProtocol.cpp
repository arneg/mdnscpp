#include "sockAddrToIPProtocol.h"

#include "throw.h"

namespace mdnscpp
{
  IPProtocol sockAddrToIPProtocol(const struct sockaddr *address)
  {
    auto family = address->sa_family;

    if (!((family == AF_INET || family == AF_INET6)))
      MDNSCPP_THROW(
          std::invalid_argument, "sockAddrToString(): Not and INET address.");

    if (address->sa_family == AF_INET)
    {
      return IPProtocol::IPv4;
    }
    else
    {
      return IPProtocol::IPv6;
    }
  }
} // namespace mdnscpp
