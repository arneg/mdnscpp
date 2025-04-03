#pragma once

#include <memory>
#include <string>

#include <mdnscpp/BrowseResult.h>
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
    enum class State : uint8_t
    {
      INIT,
      PENDING,
      DONE,
      FAILED,
    };

  public:
    Win32Resolve(std::shared_ptr<Win32Browser> browser, std::string queryName);
    ~Win32Resolve();

    void refresh(uint32_t ttl);

  private:
    static void DnsServiceResolveComplete(
        DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance);

    void onResolveResult(PDNS_SERVICE_INSTANCE pInstance);
    void cancel();
    uint64_t age() const;
    void resetUpdateTime();
    bool isFresh() const;
    void updateResult(std::shared_ptr<BrowseResult> &slot,
        std::shared_ptr<BrowseResult> result);

    std::shared_ptr<Win32Browser> browser_;
    CallQueue queue_;
    std::string queryName_;
    DNS_SERVICE_CANCEL cancel_;
    std::shared_ptr<BrowseResult> ip4Result_, ip6Result_;
    uint64_t updateTime_;
    State state_ = State::INIT;
    uint32_t ttl_;
  };
} // namespace mdnscpp