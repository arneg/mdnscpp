#pragma once

#include "Win32Resolve.h"
#include "Win32Browser.h"
#include "toWideString.h"

#include <iostream>

namespace mdnscpp
{

  Win32Resolve::Win32Resolve(
      std::shared_ptr<Win32Browser> browser, const std::string queryName)
      : browser_(browser), queryName_(queryName)
  {
    std::cerr << "Win32Resolve(" << browser->describe() << ", " << queryName
              << ")" << std::endl;

    DNS_SERVICE_RESOLVE_REQUEST request;

    std::wstring wideQueryName = toWideString(queryName);

    request.Version = DNS_QUERY_REQUEST_VERSION1;
    request.InterfaceIndex =
        static_cast<unsigned long>(browser->getInterface());
    request.QueryName = wideQueryName.data();
    request.pQueryContext = this;
    request.pResolveCompletionCallback = DnsServiceResolveComplete;

    auto status = DnsServiceResolve(&request, &cancel_);
  }

  void Win32Resolve::DnsServiceResolveComplete(
      DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance)
  {
    if (pInstance)
    {
      IPAddress ipv4{pInstance->ip4Address, pInstance->wPort};
      IPAddress ipv6{pInstance->ip6Address, pInstance->wPort};

      std::cerr << "found " << ipv4.getDecimalString() << " port"
                << ipv4.getPort() << std::endl;
      std::cerr << "found " << ipv6.getDecimalString() << " port"
                << ipv6.getPort() << std::endl;

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