#pragma once

#include <mdnscpp/Platform.h>

namespace mdnscpp
{
  class Win32Platform : public Platform,
                        public std::enable_shared_from_this<Win32Platform>
  {
  public:
    Win32Platform(EventLoop &loop);

    std::shared_ptr<Browser> createBrowser(const std::string &type,
        const std::string &protocol,
        Browser::ResultsChangedCallback onResultsChanged,
        const std::string &domain = "", size_t interfaceIndex = 0,
        IPProtocol ipProtocol = IPProtocol::Both) override;
  };
} // namespace mdnscpp
