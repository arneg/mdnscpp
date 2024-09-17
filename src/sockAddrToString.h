#pragma once

#include <arpa/inet.h>
#include <string>

namespace mdnscpp
{
  std::string sockAddrToString(const struct sockaddr *address);
}
