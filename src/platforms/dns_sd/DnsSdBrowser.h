#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <dns_sd.h>

#include "DnsSdRef.h"
#include "DnsSdResolve.h"
#include <mdnscpp/Browser.h>

namespace mdnscpp
{
  class DnsSdPlatform;

  class DnsSdBrowser : DnsSdRef,
                       public Browser,
                       public std::enable_shared_from_this<DnsSdBrowser>
  {
  public:
    DnsSdBrowser(std::shared_ptr<DnsSdPlatform> platform,
        const std::string &type, const std::string &protocol,
        std::function<void(const Browser &)> onResultsChanged,
        const std::string &domain, size_t interface);

    ~DnsSdBrowser();

    std::string describe() const;

    using DnsSdRef::getPlatform;

  private:
    std::vector<std::shared_ptr<DnsSdResolve>> resolves_;

    void browseResult(uint32_t interfaceIndex, DNSServiceErrorType errorCode,
        DNSServiceFlags flags, const char *serviceName, const char *regtype,
        const char *replyDomain);

    static DNSServiceRef startBrowse(const std::string &type,
        const std::string &protocol, const std::string &domain,
        size_t interface, void *context);
    static void browseResultCallback(DNSServiceRef sdRef, DNSServiceFlags flags,
        uint32_t interfaceIndex, DNSServiceErrorType errorCode,
        const char *serviceName, const char *regtype, const char *replyDomain,
        void *context);
  };
} // namespace mdnscpp
