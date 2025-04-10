#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <mdnscpp/Browser.h>
#include <mdnscpp/CallQueue.h>
#include <mdnscpp/IPAddress.h>

#include <Windows.h>
// windows.h comes first.
#include <WinDNS.h>

#include "Win32Resolve.h"

namespace mdnscpp
{
  class Win32Platform;

  struct Win32BrowseResult
  {
    std::string queryName;
    // This tll refers to the service itself, not
    // the TXT, A, SRV, etc.
    uint32_t ttl;
    std::vector<IPAddress> addresses;
    std::string hostname;
    uint16_t port;
  };

  class Win32Browser : public Browser,
                       public std::enable_shared_from_this<Win32Browser>
  {
  public:
    Win32Browser(std::shared_ptr<Win32Platform> platform,
        const std::string &type, const std::string &protocol,
        ResultsChangedCallback onResultsChanged, const std::string &domain,
        size_t interfaceIndex, IPProtocol ipProtocol);

    ~Win32Browser();

    std::string describe() const;
    std::shared_ptr<Win32Platform> getPlatform() const;

  protected:
    std::shared_ptr<Browser> getSharedFromThis() override;

  private:
    static void DnsQueryCompletionRoutine(
        void *pQueryContext, DNS_QUERY_RESULT *queryResults);

    void onBrowseResult(Win32BrowseResult result);
    std::string stripQueryName(std::string fullname) const;

    CallQueue queue_;
    DNS_SERVICE_CANCEL cancel_;
    std::unordered_map<std::string, std::shared_ptr<Win32Resolve>> resolves_;
    std::shared_ptr<Win32Platform> platform_;
    std::string queryName_;
  };
} // namespace mdnscpp
