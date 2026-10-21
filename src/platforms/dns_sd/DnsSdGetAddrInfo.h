#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <dns_sd.h>

#include "DnsSdRef.h"
#include <mdnscpp/Types.h>

namespace mdnscpp
{
  class DnsSdResolve;
  class DnsSdGetAddrInfo : DnsSdRef
  {
  public:
    DnsSdGetAddrInfo(std::shared_ptr<DnsSdResolve> resolve,
        size_t interfaceIndex, const std::string &hostname, uint16_t port);
    ~DnsSdGetAddrInfo();

    std::string describe() const;

    std::shared_ptr<DnsSdResolve> getResolve();

    /**
     * Replaces each result with one that has this port and the resolve's
     * current TXT records. Used when a resolve reply changes them and the
     * host stays the same.
     */
    void refresh(uint16_t port);

  private:
    struct Entry
    {
      std::string hostname;
      IPProtocol ipProtocol;
      std::shared_ptr<void> removal;
    };

    const std::weak_ptr<DnsSdResolve> resolve_;
    const size_t interfaceIndex_;
    const std::string hostname_;
    uint16_t port_;

    std::unordered_map<std::string, Entry> resultRemovalContext_;

    void addResult(const std::string &hostname, const std::string &ipAddress,
        IPProtocol ipProtocol);
    void onResult(DNSServiceFlags flags, uint32_t interfaceIndexIndex,
        DNSServiceErrorType errorCode, const char *hostname,
        const struct sockaddr *address, uint32_t ttl);
    static DNSServiceRef startGetAddrInfo(
        size_t interfaceIndex, const std::string &hostname, void *context);
    static void getAddrInfoResultCallback(DNSServiceRef sdRef,
        DNSServiceFlags flags, uint32_t interfaceIndexIndex,
        DNSServiceErrorType errorCode, const char *hostname,
        const struct sockaddr *address, uint32_t ttl, void *context);
  };
} // namespace mdnscpp
