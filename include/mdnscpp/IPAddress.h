#pragma once

#include "Types.h"

#include <cstdint>
#include <string>

#ifdef LIBMDNS_PLATFORM_WIN32
#  include <Winsock2.h>
//
#  include <Windows.h>
// always comes first.
#  include <ws2tcpip.h>
// order seems to matter
#  include <WinDNS.h>
#else
#  include <arpa/inet.h>
#endif

namespace mdnscpp
{
  class IPAddress
  {
  public:
    std::string getDecimalString() const;
    uint16_t getPort() const;
    const struct sockaddr *getSockaddr() const;
    IPProtocol getType() const;

    explicit IPAddress(const struct sockaddr *addr);

#ifdef LIBMDNS_PLATFORM_WIN32
    explicit IPAddress(const IP6_ADDRESS *addr, uint16_t port = 0);
    explicit IPAddress(const IP4_ADDRESS *addr, uint16_t port = 0);
#endif

  private:
    struct sockaddr_storage data_ = {0};
  };
} // namespace mdnscpp