#include "DnsSdGetAddrInfo.h"

#include "../../sockAddrToString.h"
#include "DnsSdPlatform.h"

#include <iostream>
#include <stdexcept>

#include <netdb.h>

#include <arpa/inet.h>

namespace mdnscpp
{
  DnsSdGetAddrInfo::DnsSdGetAddrInfo(std::shared_ptr<DnsSdPlatform> platform,
      size_t interface, const std::string &hostname)
      : DnsSdRef(platform, startGetAddrInfo(interface, hostname, this)),
        interface_(interface), hostname_(hostname)
  {
    std::cerr << describe() << "" << std::endl;
  }

  DnsSdGetAddrInfo::~DnsSdGetAddrInfo()
  {
    std::cerr << "~" << describe() << std::endl;
  }

  std::string DnsSdGetAddrInfo::describe() const
  {
    std::string result = "DnsSdGetAddrInfo(";
    result += "if ";
    result += std::to_string(interface_);
    result += ", ";
    result += hostname_;
    result += ")";
    return result;
  }

  void DnsSdGetAddrInfo::onResult(DNSServiceFlags flags,
      uint32_t interfaceIndex, DNSServiceErrorType errorCode,
      const char *hostname, const struct sockaddr *address, uint32_t ttl)
  {
    std::cerr << describe() << ": get addr info " << sockAddrToString(address)
              << std::endl;
  }

  DNSServiceRef DnsSdGetAddrInfo::startGetAddrInfo(
      size_t interface, const std::string &hostname, void *context)
  {
    DNSServiceRef sdRef;

    const auto error =
        DNSServiceGetAddrInfo(&sdRef, 0, static_cast<uint32_t>(interface),
            kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6,
            hostname.c_str(), getAddrInfoResultCallback, context);

    if (kDNSServiceErr_NoError != error)
      throw std::runtime_error("Failed.");

    return sdRef;
  }

  void DnsSdGetAddrInfo::getAddrInfoResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, const char *hostname,
      const struct sockaddr *address, uint32_t ttl, void *context)
  {
    reinterpret_cast<DnsSdGetAddrInfo *>(context)->onResult(
        flags, interfaceIndex, errorCode, hostname, address, ttl);
  }
} // namespace mdnscpp
