#pragma once

#include <arpa/inet.h>
#include <mdnscpp/Types.h>

namespace mdnscpp
{
  IPProtocol sockAddrToIPProtocol(const struct sockaddr *address);
}