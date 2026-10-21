#include "DnsSdGetAddrInfo.h"

#include "../../sockAddrToIPProtocol.h"
#include "../../sockAddrToString.h"
#include "../../throw.h"
#include "DnsSdBrowser.h"
#include "DnsSdPlatform.h"
#include "DnsSdResolve.h"

#include <mdnscpp/BrowseResult.h>

#include "../../debug.h"
#include <stdexcept>

#include <netdb.h>

#include <arpa/inet.h>

namespace mdnscpp
{
  DnsSdGetAddrInfo::DnsSdGetAddrInfo(std::shared_ptr<DnsSdResolve> resolve,
      size_t interfaceIndex, const std::string &hostname, uint16_t port)
      : DnsSdRef(resolve->getPlatform(),
            startGetAddrInfo(interfaceIndex, hostname, this)),
        resolve_(resolve), interfaceIndex_(interfaceIndex), hostname_(hostname),
        port_(port)
  {
    MDNSCPP_INFO << describe() << " created " << MDNSCPP_ENDL;
  }

  DnsSdGetAddrInfo::~DnsSdGetAddrInfo()
  {
    MDNSCPP_INFO << "~" << describe() << MDNSCPP_ENDL;
  }

  std::string DnsSdGetAddrInfo::describe() const
  {
    std::string result = "DnsSdGetAddrInfo(";
    result += "if ";
    result += std::to_string(interfaceIndex_);
    result += ", ";
    result += hostname_;
    result += ")";
    return result;
  }

  std::shared_ptr<DnsSdResolve> DnsSdGetAddrInfo::getResolve()
  {
    auto result = resolve_.lock();
    if (!result)
      MDNSCPP_THROW(std::logic_error,
          "DnsSdGetAddrInfo() detached from parent DnsSdGetResolve().");
    return result;
  }

  void DnsSdGetAddrInfo::onResult(DNSServiceFlags flags,
      uint32_t interfaceIndexIndex, DNSServiceErrorType errorCode,
      const char *hostname, const struct sockaddr *address, uint32_t ttl)
  {
    // kDNSServiceErr_NoSuchRecord is the usual reply for a host without an
    // address of one family. Error replies carry no usable address.
    if (errorCode != kDNSServiceErr_NoError || !address)
    {
      MDNSCPP_INFO << describe() << ".onResult() error " << errorCode
                   << MDNSCPP_ENDL;
      return;
    }

    auto ipAddress = sockAddrToString(address);
    MDNSCPP_INFO << describe() << ".onResult(" << ipAddress << ")"
                 << MDNSCPP_ENDL;

    if (ipAddress.empty())
      return;

    if (!(flags & kDNSServiceFlagsAdd))
    {
      resultRemovalContext_.erase(ipAddress);
      return;
    }

    auto resolve = getResolve();
    auto browser = resolve->getBrowser();

    resultRemovalContext_[ipAddress] = browser->addResult(
        std::make_shared<BrowseResult>(resolve->getTxtRecords(),
            browser->getType(), browser->getProtocol(), resolve->getName(),
            resolve->getDomain(), hostname, ipAddress, resolve->getInterface(),
            sockAddrToIPProtocol(address), port_));
  }

  DNSServiceRef DnsSdGetAddrInfo::startGetAddrInfo(
      size_t interfaceIndex, const std::string &hostname, void *context)
  {
    DNSServiceRef sdRef = nullptr;

    const auto error =
        DNSServiceGetAddrInfo(&sdRef, 0, static_cast<uint32_t>(interfaceIndex),
            kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6,
            hostname.c_str(), getAddrInfoResultCallback, context);

    if (kDNSServiceErr_NoError != error)
      MDNSCPP_THROW(std::runtime_error, "Failed.");

    return sdRef;
  }

  void DnsSdGetAddrInfo::getAddrInfoResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndexIndex,
      DNSServiceErrorType errorCode, const char *hostname,
      const struct sockaddr *address, uint32_t ttl, void *context)
  {
    // An exception must not unwind through DNSServiceProcessResult().
#ifdef __cpp_exceptions
    try
    {
#endif
      reinterpret_cast<DnsSdGetAddrInfo *>(context)->onResult(
          flags, interfaceIndexIndex, errorCode, hostname, address, ttl);
#ifdef __cpp_exceptions
    }
    catch (const std::exception &e)
    {
      MDNSCPP_ERROR << "getAddrInfoResultCallback() threw: " << e.what()
                    << MDNSCPP_ENDL;
    }
    catch (...)
    {
      MDNSCPP_ERROR << "getAddrInfoResultCallback() threw." << MDNSCPP_ENDL;
    }
#endif
  }
} // namespace mdnscpp
