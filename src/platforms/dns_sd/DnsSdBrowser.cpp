#include "DnsSdBrowser.h"

#include <iostream>
#include <stdexcept>

#include <dns_sd.h>

#include "DnsSdPlatform.h"

namespace mdnscpp
{
  DnsSdBrowser::DnsSdBrowser(std::shared_ptr<DnsSdPlatform> platform,
      const std::string &type, const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interface)
      : DnsSdRef(
            platform, startBrowse(type, protocol, domain, interface, this)),
        Browser(type, protocol, onResultsChanged, domain, interface)
  {
  }

  DnsSdBrowser::~DnsSdBrowser() { std::cerr << "~DnsSdBrowser()" << std::endl; }

  std::string DnsSdBrowser::describe() const
  {
    std::string result = "DnsSdBrowse(";
    result += type_;
    result += ", ";
    result += protocol_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += interface_;
    result += ")";
    return result;
  }

  DNSServiceRef DnsSdBrowser::startBrowse(const std::string &type,
      const std::string &protocol, const std::string &domain, size_t interface,
      void *context)
  {
    DNSServiceRef sdRef = nullptr;
    std::string regType = type + "." + protocol;

    auto error = DNSServiceBrowse(&sdRef, 0, static_cast<uint32_t>(interface),
        regType.c_str(), domain.size() ? domain.c_str() : nullptr,
        browseResultCallback, context);

    if (kDNSServiceErr_NoError != error)
      throw std::runtime_error("Failed.");

    return sdRef;
  }

  void DnsSdBrowser::browseResult(uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, DNSServiceFlags flags,
      const char *serviceName, const char *regtype, const char *replyDomain)
  {
    if (kDNSServiceErr_NoError != errorCode)
    {
      std::cerr << describe() << " Service browsing failed with err "
                << errorCode << std::endl;
      return;
    }

    if (flags & kDNSServiceFlagsAdd)
    {
      std::cerr << describe() << " Service found " << serviceName << std::endl;
      resolves_.push_back(std::make_shared<DnsSdResolve>(shared_from_this(),
          interfaceIndex, serviceName, regtype, replyDomain));
    }
    else
    {
      std::cerr << describe() << " Service removed " << serviceName
                << std::endl;
    }
  }

  void DnsSdBrowser::browseResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, const char *serviceName,
      const char *regtype, const char *replyDomain, void *context)
  {
    reinterpret_cast<DnsSdBrowser *>(context)->browseResult(
        interfaceIndex, errorCode, flags, serviceName, regtype, replyDomain);
  }
} // namespace mdnscpp
