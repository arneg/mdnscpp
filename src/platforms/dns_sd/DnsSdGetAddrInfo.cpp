#include "DnsSdGetAddrInfo.h"

#include "../../sockAddrToIPProtocol.h"
#include "../../sockAddrToString.h"
#include "../../throw.h"
#include "DnsSdBrowser.h"
#include "DnsSdPlatform.h"
#include "DnsSdResolve.h"

#include <mdnscpp/BrowseResult.h>

#include <iostream>
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
    std::cerr << describe() << " created " << std::endl;
  }

  DnsSdGetAddrInfo::~DnsSdGetAddrInfo()
  {
    std::cerr << "~" << describe() << std::endl;
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
    auto ipAddress = sockAddrToString(address);
    std::cerr << describe() << ".onResult(" << ipAddress << ")" << std::endl;

    auto resolve = getResolve();
    auto browser = resolve->getBrowser();

    std::vector<TxtRecord> txtRecords;

    resultRemovalContext_[ipAddress] = browser->addResult(
        std::make_shared<BrowseResult>(resolve->getTxtRecords(),
            browser->getType(), browser->getProtocol(), resolve->getName(),
            resolve->getDomain(), hostname, ipAddress, resolve->getInterface(),
            sockAddrToIPProtocol(address), port_));
  }

  DNSServiceRef DnsSdGetAddrInfo::startGetAddrInfo(
      size_t interfaceIndex, const std::string &hostname, void *context)
  {
    DNSServiceRef sdRef;

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
    reinterpret_cast<DnsSdGetAddrInfo *>(context)->onResult(
        flags, interfaceIndexIndex, errorCode, hostname, address, ttl);
  }
} // namespace mdnscpp
