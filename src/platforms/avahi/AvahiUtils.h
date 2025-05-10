#pragma once

#include <string>

#include <avahi-common/address.h>
#include <mdnscpp/Types.h>

namespace mdnscpp
{
  AvahiProtocol toAvahiProtocol(IPProtocol ipProtocol);
  IPProtocol fromAvahiProtocol(AvahiProtocol protocol);
  std::string fromAvahiAddress(const AvahiAddress *a);
  size_t fromAvahiInterfaceIndex(AvahiIfIndex index);
  AvahiIfIndex toAvahiInterfaceIndex(size_t interfaceIndex);
} // namespace mdnscpp
