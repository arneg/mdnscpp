#pragma once

#include <memory>
#include <string>

#include <dns_sd.h>

#include "DnsSdRef.h"

namespace mdnscpp
{
  class DnsSdPlatform;
  class DnsSdGetAddrInfo : DnsSdRef
  {
  public:
    DnsSdGetAddrInfo(std::shared_ptr<DnsSdPlatform> platform, size_t interface,
        const std::string &hostname);
    ~DnsSdGetAddrInfo();

    std::string describe() const;

  private:
    const size_t interface_;
    const std::string hostname_;

    void onResult(DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char *hostname,
        const struct sockaddr *address, uint32_t ttl);
    static DNSServiceRef startGetAddrInfo(
        size_t interface, const std::string &hostname, void *context);
    static void getAddrInfoResultCallback(DNSServiceRef sdRef,
        DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char *hostname,
        const struct sockaddr *address, uint32_t ttl, void *context);
  };
} // namespace mdnscpp
