#pragma once

#ifdef LIBMDNS_PLATFORM_WIN32
#  include <Winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#endif

#include <mdnscpp/Types.h>

namespace mdnscpp
{
  IPProtocol sockAddrToIPProtocol(const struct sockaddr *address);
}