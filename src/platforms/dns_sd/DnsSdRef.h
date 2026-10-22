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

    // False if the underlying DNSServiceRef was never created or has been
    // closed. A failed DNSService* call leaves the object in this state.
    bool ok() const { return sdRef_ != nullptr; }

  protected:
    std::shared_ptr<DnsSdPlatform> platform_;
    std::shared_ptr<EventLoop::Watch> watch_;
    DNSServiceRef sdRef_ = nullptr;

  private:
    void process();
  };
} // namespace mdnscpp
