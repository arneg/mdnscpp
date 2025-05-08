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
    Win32Resolve(
        Win32Browser &browser, std::string queryName, std::string name);
    ~Win32Resolve();

    void refresh(uint32_t ttl = 0);
    void resetTtl(uint32_t ttl);
    bool isGone() const;
    std::string describe() const;

  private:
    static void DnsServiceResolveComplete(
        DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance);

    void onResolveResult(PDNS_SERVICE_INSTANCE pInstance);
    void cancel();
    uint64_t resolveAge() const;
    uint64_t resultAge() const;
    void resetResultTime();
    void resetResolveTime();
    bool isFresh() const;
    bool isStale() const;
    void updateResult(std::shared_ptr<BrowseResult> &slot,
        std::shared_ptr<BrowseResult> result);
    uint64_t now() const;
    bool hasResult() const;

    Win32Browser &browser_;
    CallQueue queue_;
    std::string queryName_;
    std::string name_;
    DNS_SERVICE_CANCEL cancel_;
    std::shared_ptr<BrowseResult> ip4Result_, ip6Result_;
    uint64_t resolveTime_ = 0, resultTime_ = 0, ttlTime_ = 0;
    State state_ = State::INIT;
  };
} // namespace mdnscpp
