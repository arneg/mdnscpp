#pragma once

#include <memory>
#include <string>

#include <dns_sd.h>

#include "DnsSdGetAddrInfo.h"
#include "DnsSdRef.h"
#include <mdnscpp/TxtRecord.h>

namespace mdnscpp
{
  class DnsSdBrowser;
  class DnsSdResolve : DnsSdRef,
                       public std::enable_shared_from_this<DnsSdResolve>
  {
  public:
    DnsSdResolve(std::shared_ptr<DnsSdBrowser> browser, size_t interfaceIndex,
        const std::string &name, const std::string &type,
        const std::string &domain);
    ~DnsSdResolve();

    std::string describe() const;
    std::shared_ptr<DnsSdBrowser> getBrowser() const;
    using DnsSdRef::getPlatform;

    const std::string &getName() const;
    const std::string &getType() const;
    const std::string &getDomain() const;
    size_t getInterface() const;
    const std::vector<TxtRecord> getTxtRecords() const;

  private:
    const std::weak_ptr<DnsSdBrowser> browser_;
    const size_t interfaceIndex_;
    const std::string name_;
    const std::string type_;
    const std::string domain_;
    std::vector<TxtRecord> txtRecords_;

    std::shared_ptr<DnsSdGetAddrInfo> getaddrinfo_;

    void onResult(DNSServiceFlags flags, uint32_t interfaceIndexIndex,
        DNSServiceErrorType errorCode, const char *fullname,
        const char *hosttarget, uint16_t port, uint16_t txtLen,
        const unsigned char *txtRecord);
    static DNSServiceRef startResolve(size_t interfaceIndex,
        const std::string &name, const std::string &type,
        const std::string &domain, void *context);
    static void resolveResultCallback(DNSServiceRef sdRef,
        DNSServiceFlags flags, uint32_t interfaceIndexIndex,
        DNSServiceErrorType errorCode, const char *fullname,
        const char *hosttarget, uint16_t port, uint16_t txtLen,
        const unsigned char *txtRecord, void *context);
  };
} // namespace mdnscpp
