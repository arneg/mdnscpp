#pragma once

#include <memory>
#include <string>

#include <WinSock2.h>
#include <Windows.h>
// windows.h comes first.
#include <WinDNS.h>

namespace mdnscpp
{
  class Win32Browser;
  class Win32Resolve
  {
  public:
    Win32Resolve(std::shared_ptr<Win32Browser> browser, std::string queryName);
    ~Win32Resolve();

  private:
    static void DnsServiceResolveComplete(
        DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance);

    std::shared_ptr<Win32Browser> browser_;
    std::string queryName_;
    DNS_SERVICE_CANCEL cancel_;
  };
} // namespace mdnscpp