#pragma once

#ifdef LIBMDNS_PLATFORM_WIN32
#  include <Winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#endif

#include <string>

namespace mdnscpp
{
  std::string sockAddrToString(const struct sockaddr *address);
}
