#pragma once

#include "Win32Resolve.h"
#include "Win32Browser.h"
#include "Win32Platform.h"
#include "fromWideString.h"
#include "toWideString.h"

#include <iostream>

namespace mdnscpp
{

  Win32Resolve::Win32Resolve(
      std::shared_ptr<Win32Browser> browser, const std::string queryName)
      : browser_(browser), queryName_(queryName),
        queue_(browser->getPlatform()->getEventLoop())
  {
    std::cerr << "Win32Resolve(" << browser->describe() << ", " << queryName
              << ")" << std::endl;

    DNS_SERVICE_RESOLVE_REQUEST request;

    std::wstring wideQueryName = toWideString(queryName);

    request.Version = DNS_QUERY_REQUEST_VERSION1;
    request.InterfaceIndex = 13;
    //static_cast<unsigned long>(browser->getInterface());
    request.QueryName = wideQueryName.data();
    request.pQueryContext = this;
    request.pResolveCompletionCallback = DnsServiceResolveComplete;

    std::cerr << "Calling DnsServiceResolve." << std::endl;
    auto status = DnsServiceResolve(&request, &cancel_);
  }

  void Win32Resolve::onResolveResult(PDNS_SERVICE_INSTANCE pInstance)
  {
    IPAddress ipv4{pInstance->ip4Address, pInstance->wPort};
    IPAddress ipv6{pInstance->ip6Address, pInstance->wPort};

    std::cerr << "found " << ipv4.getDecimalString() << " port"
              << ipv4.getPort() << std::endl;
    std::cerr << "found " << ipv6.getDecimalString() << " port"
              << ipv6.getPort() << std::endl;

    std::vector<TxtRecord> txtRecords;
    std::string hostname = fromWideString(pInstance->pszHostName);
    size_t interfaceIndex = pInstance->dwInterfaceIndex;
    std::string instanceName = fromWideString(pInstance->pszInstanceName);
    uint16_t port = pInstance->wPort;

    for (DWORD i = 0; i < pInstance->dwPropertyCount; i++)
    {
      TxtRecord record;

      record.key = fromWideString(pInstance->keys[i]);
      record.value = fromWideString(pInstance->values[i]);
    }

    queue_.schedule([ipv4, ipv6, txtRecords, hostname, port, interfaceIndex,
                        instanceName, this]() {
      auto result = std::make_shared<BrowseResult>(txtRecords,
          browser_->getType(), browser_->getProtocol(), instanceName,
          browser_->getDomain(), hostname, ipv4.getDecimalString(),
          interfaceIndex, IPProtocol::IPv4);

      removals_.push_back(browser_->addResult(std::move(result)));
    });
  }

  void Win32Resolve::DnsServiceResolveComplete(
      DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance)
  {
    if (pInstance)
    {
      Win32Resolve *self = reinterpret_cast<Win32Resolve *>(pQueryContext);
      self->onResolveResult(pInstance);
      DnsServiceFreeInstance(pInstance);
    }
  }

  Win32Resolve::~Win32Resolve()
  {
    std::cerr << "~Win32Resolve(" << browser_->describe() << ", " << queryName_
              << ")" << std::endl;
    DnsServiceResolveCancel(&cancel_);
  }
} // namespace mdnscpp