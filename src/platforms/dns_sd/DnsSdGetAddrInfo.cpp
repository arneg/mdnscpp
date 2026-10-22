#include "DnsSdGetAddrInfo.h"

#include "../../sockAddrToIPProtocol.h"
#include "../../sockAddrToString.h"
#include "DnsSdBrowser.h"
#include "DnsSdPlatform.h"
#include "DnsSdResolve.h"

#include <mdnscpp/BrowseResult.h>

#include "../../debug.h"
#include "../../guardCallback.h"

#include <netdb.h>

#include <arpa/inet.h>

namespace mdnscpp
{
  static DNSServiceProtocol getDnsServiceProtocol(IPProtocol proto)
  {
    switch (proto)
    {
    case IPProtocol::IPv4:
      return kDNSServiceProtocol_IPv4;
    case IPProtocol::IPv6:
      return kDNSServiceProtocol_IPv6;
    case IPProtocol::Both:
      return kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6;
    }

    return 0;
  }

  static bool isInetAddress(const struct sockaddr *address)
  {
    return address &&
           (address->sa_family == AF_INET || address->sa_family == AF_INET6);
  }

  DnsSdGetAddrInfo::DnsSdGetAddrInfo(std::shared_ptr<DnsSdResolve> resolve,
      size_t interfaceIndex, const std::string &hostname, uint16_t port)
      : DnsSdRef(resolve->getPlatform(), startGetAddrInfoFromResolve(resolve,
                                             interfaceIndex, hostname, this)),
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
    return resolve_.lock();
  }

  void DnsSdGetAddrInfo::onResult(DNSServiceFlags flags,
      uint32_t interfaceIndexIndex, DNSServiceErrorType errorCode,
      const char *hostname, const struct sockaddr *address, uint32_t ttl)
  {
    // Apple documents the other parameters as undefined when errorCode is
    // non-zero (e.g. a host with no AAAA record).
    if (errorCode != kDNSServiceErr_NoError)
    {
      MDNSCPP_ERROR << describe() << ".onResult() failed with error "
                    << errorCode << MDNSCPP_ENDL;
      return;
    }

    if (!isInetAddress(address))
    {
      MDNSCPP_ERROR << describe()
                    << ".onResult() ignored: missing or non-IP address."
                    << MDNSCPP_ENDL;
      return;
    }

    auto ipAddress = sockAddrToString(address);
    MDNSCPP_INFO << describe() << ".onResult(" << ipAddress << ")"
                 << MDNSCPP_ENDL;

    // Without kDNSServiceFlagsAdd the address is no longer valid. Dropping
    // its removal context removes the corresponding result.
    if (!(flags & kDNSServiceFlagsAdd))
    {
      resultRemovalContext_.erase(ipAddress);
      return;
    }

    if (!hostname)
    {
      MDNSCPP_ERROR << describe() << ".onResult() ignored: missing hostname."
                    << MDNSCPP_ENDL;
      return;
    }

    auto resolve = getResolve();
    if (!resolve)
      return;

    auto browser = resolve->getBrowser();
    if (!browser)
      return;

    resultRemovalContext_[ipAddress] = browser->addResult(
        std::make_shared<BrowseResult>(resolve->getTxtRecords(),
            browser->getType(), browser->getProtocol(), resolve->getName(),
            resolve->getDomain(), hostname, ipAddress, resolve->getInterface(),
            sockAddrToIPProtocol(address), port_));
  }

  DNSServiceRef DnsSdGetAddrInfo::startGetAddrInfo(size_t interfaceIndex,
      DNSServiceProtocol protocol, const std::string &hostname, void *context)
  {
    DNSServiceRef sdRef = nullptr;

    const auto error =
        DNSServiceGetAddrInfo(&sdRef, 0, static_cast<uint32_t>(interfaceIndex),
            protocol, hostname.c_str(), getAddrInfoResultCallback, context);

    if (kDNSServiceErr_NoError != error)
    {
      MDNSCPP_ERROR << "DNSServiceGetAddrInfo(" << hostname
                    << ") failed with error " << error << MDNSCPP_ENDL;
      return nullptr;
    }

    return sdRef;
  }

  DNSServiceRef DnsSdGetAddrInfo::startGetAddrInfoFromResolve(
      const std::shared_ptr<DnsSdResolve> &resolve, size_t interfaceIndex,
      const std::string &hostname, void *context)
  {
    auto browser = resolve ? resolve->getBrowser() : nullptr;
    if (!browser)
    {
      MDNSCPP_ERROR << "DnsSdGetAddrInfo(" << hostname
                    << ") cannot start: parent browser is gone."
                    << MDNSCPP_ENDL;
      return nullptr;
    }

    return startGetAddrInfo(interfaceIndex,
        getDnsServiceProtocol(browser->getIPProtocol()), hostname, context);
  }

  void DnsSdGetAddrInfo::getAddrInfoResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndexIndex,
      DNSServiceErrorType errorCode, const char *hostname,
      const struct sockaddr *address, uint32_t ttl, void *context)
  {
    guardCallback("DnsSdGetAddrInfo::getAddrInfoResultCallback", [&] {
      reinterpret_cast<DnsSdGetAddrInfo *>(context)->onResult(
          flags, interfaceIndexIndex, errorCode, hostname, address, ttl);
    });
  }
} // namespace mdnscpp
