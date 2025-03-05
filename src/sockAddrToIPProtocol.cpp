#include "sockAddrToIPProtocol.h"

#include <stdexcept>

namespace mdnscpp
{
  IPProtocol sockAddrToIPProtocol(const struct sockaddr *address)
  {
    if (address->sa_family == AF_INET)
    {
      return IPProtocol::IPv4;
    }
    else if (address->sa_family == AF_INET6)
    {
      return IPProtocol::IPv6;
    }
    else
    {
      throw std::invalid_argument("sockAddrToString(): Not and INET address.");
    }
  }
} // namespace mdnscpp
