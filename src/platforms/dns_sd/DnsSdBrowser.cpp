#include "DnsSdBrowser.h"

#include "../../debug.h"
#include <stdexcept>

#include <dns_sd.h>

#include "../../debug.h"
#include "../../throw.h"
#include "DnsSdPlatform.h"

namespace mdnscpp
{
  DnsSdBrowser::DnsSdBrowser(std::shared_ptr<DnsSdPlatform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : DnsSdRef(platform,
            startBrowse(type, protocol, domain, interfaceIndex, this)),
        Browser(type, protocol, onResultsChanged, domain, interfaceIndex,
            ipProtocol)
  {
  }

  DnsSdBrowser::~DnsSdBrowser()
  {
    MDNSCPP_INFO << "~DnsSdBrowser()" << MDNSCPP_ENDL;
    onResultsChanged_ = nullptr;
    resolves_.clear();
  }

  std::string DnsSdBrowser::describe() const
  {
    std::string result = "DnsSdBrowse(";
    result += type_;
    result += ", ";
    result += protocol_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += interfaceIndex_;
    result += ")";
    return result;
  }

  DNSServiceRef DnsSdBrowser::startBrowse(const std::string &type,
      const std::string &protocol, const std::string &domain,
      size_t interfaceIndex, void *context)
  {
    DNSServiceRef sdRef = nullptr;
    std::string regType = type + "." + protocol;

    auto error =
        DNSServiceBrowse(&sdRef, 0, static_cast<uint32_t>(interfaceIndex),
            regType.c_str(), domain.size() ? domain.c_str() : nullptr,
            browseResultCallback, context);

    if (kDNSServiceErr_NoError != error)
      MDNSCPP_THROW(std::runtime_error, "Failed.");

    return sdRef;
  }

  void DnsSdBrowser::browseResult(uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, DNSServiceFlags flags,
      const char *serviceName, const char *regtype, const char *replyDomain)
  {
    if (kDNSServiceErr_NoError != errorCode)
    {
      MDNSCPP_ERROR << describe() << " Service browsing failed with err "
                    << errorCode << MDNSCPP_ENDL;
      return;
    }

    std::string key;

    key.reserve(32);

    key += std::to_string(interfaceIndex);
    key += ",";
    key += regtype;
    key += replyDomain;
    key += ",";
    key += serviceName;

    if (flags & kDNSServiceFlagsAdd)
    {
      MDNSCPP_INFO << describe() << " Service found "
                   << "serviceName " << serviceName << ", "
                   << "regtype " << regtype << ", "
                   << "replyDomain " << replyDomain << MDNSCPP_ENDL;

      resolves_[key] = std::make_shared<DnsSdResolve>(shared_from_this(),
          interfaceIndex, serviceName, regtype, replyDomain);
    }
    else
    {
      resolves_.erase(key);
      MDNSCPP_INFO << describe() << " Service removed "
                   << "serviceName " << serviceName << ", "
                   << "regtype " << regtype << ", "
                   << "replyDomain " << replyDomain << MDNSCPP_ENDL;
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

  std::shared_ptr<Browser> DnsSdBrowser::getSharedFromThis()
  {
    return shared_from_this();
  }
} // namespace mdnscpp
