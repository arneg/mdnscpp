#include "AvahiUtils.h"

namespace mdnscpp
{
  AvahiProtocol toAvahiProtocol(IPProtocol ipProtocol)
  {
    switch (ipProtocol)
    {
    case IPProtocol::IPv4:
      return AVAHI_PROTO_INET;
    case IPProtocol::IPv6:
      return AVAHI_PROTO_INET6;
    default:
      return AVAHI_PROTO_UNSPEC;
    }
  }

  IPProtocol fromAvahiProtocol(AvahiProtocol protocol)
  {
    switch (protocol)
    {
    case AVAHI_PROTO_INET:
      return IPProtocol::IPv4;
    case AVAHI_PROTO_INET6:
      return IPProtocol::IPv6;
    default:
      return IPProtocol::Both;
    }
  }

  std::string fromAvahiAddress(const AvahiAddress *a)
  {
    std::string result;

    result.resize(AVAHI_ADDRESS_STR_MAX);

    avahi_address_snprint(result.data(), result.size(), a);

    size_t len = result.size();

    while (len > 0 && result[len - 1] == 0)
      len--;

    result.resize(len);

    return result;
  }

  size_t fromAvahiInterfaceIndex(AvahiIfIndex index)
  {
    if (index == AVAHI_IF_UNSPEC)
    {
      return 0;
    }

    return static_cast<size_t>(index);
  }

  AvahiIfIndex toAvahiInterfaceIndex(size_t interfaceIndex)
  {
    if (!interfaceIndex)
    {
      return AVAHI_IF_UNSPEC;
    }

    return static_cast<AvahiIfIndex>(interfaceIndex);
  }
} // namespace mdnscpp
