#include "sockAddrToString.h"

#ifdef __cpp_exception
#  include <stdexcept>
#endif

static inline void resizeToNullByte(std::string &str)
{
  const auto pos = str.find_first_of('\0');

  if (pos == std::string::npos)
    return;

  str.resize(pos);
}

namespace mdnscpp
{

  std::string sockAddrToString(const struct sockaddr *address)
  {
    std::string result;

    socklen_t len;
    const void *src;

    if (address->sa_family == AF_INET)
    {
      const struct sockaddr_in *addr_in =
          reinterpret_cast<const struct sockaddr_in *>(address);
      len = INET_ADDRSTRLEN;
      src = &(addr_in->sin_addr);
    }
    else if (address->sa_family == AF_INET6)
    {
      const struct sockaddr_in6 *addr_in =
          reinterpret_cast<const struct sockaddr_in6 *>(address);
      len = INET6_ADDRSTRLEN;
      src = &(addr_in->sin6_addr);
    }
    else
    {
#ifdef __cpp_exception
      throw std::invalid_argument("sockAddrToString(): Not and INET address.");
#else
      return "";
#endif
    }

    result.resize(len);

    const char *s = inet_ntop(address->sa_family, src, result.data(), len);

    if (!s)
    {
#ifdef __cpp_exception
      throw std::invalid_argument(
          "sockAddrToString(): Failed to print ip address.");
#else
      return "";
#endif
    }

    resizeToNullByte(result);

    return result;
  }

} // namespace mdnscpp
