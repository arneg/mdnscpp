#pragma once

#include <memory>
#include <string>

#include <dns_sd.h>

#include "DnsSdRef.h"

namespace mdnscpp
{
  class DnsSdPlatform;
  class DnsSdResolve : DnsSdRef
  {
  public:
    DnsSdResolve(std::shared_ptr<DnsSdPlatform> platform, size_t interface,
        const std::string &name, const std::string &type,
        const std::string &domain);
    ~DnsSdResolve();

    std::string describe() const;

  private:
    const size_t interface_;
    const std::string name_;
    const std::string type_;
    const std::string domain_;

    void onResult(DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char *fullname,
        const char *hosttarget, uint16_t port, uint16_t txtLen,
        const unsigned char *txtRecord);
    static DNSServiceRef startResolve(size_t interface, const std::string &name,
        const std::string &type, const std::string &domain, void *context);
    static void resolveResultCallback(DNSServiceRef sdRef,
        DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char *fullname,
        const char *hosttarget, uint16_t port, uint16_t txtLen,
        const unsigned char *txtRecord, void *context);
  };
} // namespace mdnscpp
