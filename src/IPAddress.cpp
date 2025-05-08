#include <mdnscpp/IPAddress.h>

#include "sockAddrToString.h"
#include "throw.h"

#include <stdexcept>

namespace mdnscpp
{

  std::string IPAddress::getDecimalString() const
  {
    return sockAddrToString(getSockaddr());
  }

  uint16_t IPAddress::getPort() const
  {
    switch (data_.ss_family)
    {
    case AF_INET: {
      const struct sockaddr_in *tmp =
          reinterpret_cast<const struct sockaddr_in *>(&data_);
      return tmp->sin_port;
    }

    case AF_INET6: {
      const struct sockaddr_in6 *tmp =
          reinterpret_cast<const struct sockaddr_in6 *>(&data_);
      return tmp->sin6_port;
    }

    default:
      return 0;
      // should never happen
    }
  }

  const struct sockaddr *IPAddress::getSockaddr() const
  {
    return reinterpret_cast<const struct sockaddr *>(&data_);
  }

  IPProtocol IPAddress::getType() const
  {
    switch (data_.ss_family)
    {
    default:
      // should never happen
    case AF_INET:
      return IPProtocol::IPv4;
    case AF_INET6:
      return IPProtocol::IPv6;
    }
  }

  IPAddress::IPAddress(const struct sockaddr *addr)
  {
    switch (addr->sa_family)
    {
    case AF_INET:
      memcpy(&data_, reinterpret_cast<const struct sockaddr_in *>(addr),
          sizeof(struct sockaddr_in));
      break;
    case AF_INET6:
      memcpy(&data_, reinterpret_cast<const struct sockaddr_in6 *>(addr),
          sizeof(struct sockaddr_in6));
      break;
    default:
      MDNSCPP_THROW(std::runtime_error, "Unsupported ip family.");
    }
  }
#ifdef LIBMDNS_PLATFORM_WIN32
  IPAddress::IPAddress(const IP6_ADDRESS *addr, uint16_t port)
  {
    struct sockaddr_in6 *tmp = reinterpret_cast<struct sockaddr_in6 *>(&data_);
    tmp->sin6_family = AF_INET6;
    tmp->sin6_port = port;
    memcpy(&tmp->sin6_addr, addr, sizeof(tmp->sin6_addr));
  }

  IPAddress::IPAddress(const IP4_ADDRESS *addr, uint16_t port)
  {
    struct sockaddr_in *tmp = reinterpret_cast<struct sockaddr_in *>(&data_);
    tmp->sin_family = AF_INET;
    tmp->sin_port = port;
    tmp->sin_addr.S_un.S_addr = *addr;
  }
#endif
} // namespace mdnscpp
