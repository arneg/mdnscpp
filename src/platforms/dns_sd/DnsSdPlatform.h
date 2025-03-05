#pragma once

#include <unordered_map>
#include <utility>

#include <dns_sd.h>

#include <mdnscpp/Platform.h>

#include "DnsSdRef.h"

namespace mdnscpp
{
  class DnsSdPlatform : public Platform,
                        public std::enable_shared_from_this<DnsSdPlatform>
  {
  public:
    DnsSdPlatform(EventLoop &loop);

    std::shared_ptr<Browser> createBrowser(const std::string &type,
        const std::string &protocol,
        Browser::ResultsChangedCallback onResultsChanged,
        const std::string &domain = "", size_t interface = 0,
        IPProtocol ipProtocol = IPProtocol::Both) override;
  };
} // namespace mdnscpp
