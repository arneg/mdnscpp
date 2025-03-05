#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <dns_sd.h>

#include "DnsSdRef.h"

namespace mdnscpp
{
  class DnsSdResolve;
  class DnsSdGetAddrInfo : DnsSdRef
  {
  public:
    DnsSdGetAddrInfo(std::shared_ptr<DnsSdResolve> resolve, size_t interface,
        const std::string &hostname);
    ~DnsSdGetAddrInfo();

    std::string describe() const;

    std::shared_ptr<DnsSdResolve> getResolve();

  private:
    const std::weak_ptr<DnsSdResolve> resolve_;
    const size_t interface_;
    const std::string hostname_;

    std::unordered_map<std::string, std::shared_ptr<void>>
        resultRemovalContext_;

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
