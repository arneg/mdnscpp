#pragma once

#include <memory>

#include <dns_sd.h>

#include <mdnscpp/EventLoop.h>

namespace mdnscpp
{
  class DnsSdPlatform;

  class DnsSdRef
  {
  public:
    DnsSdRef(std::shared_ptr<DnsSdPlatform> platform, DNSServiceRef sdRef);
    ~DnsSdRef();

    void close();

    std::shared_ptr<DnsSdPlatform> getPlatform() const;

  protected:
    std::shared_ptr<DnsSdPlatform> platform_;
    std::shared_ptr<EventLoop::Watch> watch_;
    DNSServiceRef sdRef_ = nullptr;

  private:
    void process();
  };
} // namespace mdnscpp
