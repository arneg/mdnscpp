#include "TestHarness.h"

#include "sockAddrToString.h"

#include <cstring>
#include <stdexcept>

using namespace mdnscpp;

TEST_CASE(sockAddrToString_ipv4)
{
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  CHECK(inet_pton(AF_INET, "192.168.1.42", &addr.sin_addr) == 1);

  CHECK(sockAddrToString(reinterpret_cast<const sockaddr *>(&addr)) ==
        "192.168.1.42");
}

TEST_CASE(sockAddrToString_ipv6)
{
  sockaddr_in6 addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  CHECK(inet_pton(AF_INET6, "fe80::1", &addr.sin6_addr) == 1);

  CHECK(
      sockAddrToString(reinterpret_cast<const sockaddr *>(&addr)) == "fe80::1");
}

TEST_CASE(sockAddrToString_ipv6_has_no_trailing_nul)
{
  sockaddr_in6 addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  CHECK(inet_pton(AF_INET6, "::1", &addr.sin6_addr) == 1);

  const std::string result =
      sockAddrToString(reinterpret_cast<const sockaddr *>(&addr));
  CHECK(result == "::1");
  CHECK(result.size() == 3);
}

// Unsupported address families: behavior depends on the build, see
// sockAddrToString.cpp.
TEST_CASE(sockAddrToString_unsupported_family)
{
  sockaddr addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sa_family = AF_UNSPEC;

#ifdef __cpp_exceptions
  bool threw = false;
  try
  {
    sockAddrToString(&addr);
  }
  catch (const std::invalid_argument &)
  {
    threw = true;
  }
  CHECK(threw);
#else
  CHECK(sockAddrToString(&addr).empty());
#endif
}
