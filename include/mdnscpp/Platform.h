#pragma once

#include <functional>
#include <memory>
#include <string>

#include "Browser.h"
#include "EventLoop.h"

namespace mdnscpp
{
  class Platform
  {
  public:
    Platform(EventLoop &loop);
    virtual ~Platform() = default;

    virtual std::shared_ptr<Browser> createBrowser(const std::string &type,
        const std::string &protocol,
        Browser::ResultsChangedCallback onResultsChanged,
        const std::string &domain = "", size_t interface = 0,
        IPProtocol ipProtocol = IPProtocol::Both) = 0;
    EventLoop &getEventLoop() const;

  protected:
    EventLoop &loop_;
  };

  std::shared_ptr<Platform> createPlatform(EventLoop &loop);
} // namespace mdnscpp
