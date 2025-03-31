#pragma once

#include <memory>
#include <string>

#include <mdnscpp/CallQueue.h>

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

    void onResolveResult(PDNS_SERVICE_INSTANCE pInstance);

    std::shared_ptr<Win32Browser> browser_;
    CallQueue queue_;
    std::string queryName_;
    DNS_SERVICE_CANCEL cancel_;
    std::vector<std::shared_ptr<void>> removals_;
  };
} // namespace mdnscpp