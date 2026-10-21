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
    virtual ~DnsSdRef();

    void close();

    std::shared_ptr<DnsSdPlatform> getPlatform() const;

    // False if the underlying DNSServiceRef was never created or has been
    // closed. A failed DNSService* call leaves the object in this state.
    bool ok() const { return sdRef_ != nullptr; }

  protected:
    std::shared_ptr<DnsSdPlatform> platform_;
    std::shared_ptr<EventLoop::Watch> watch_;
    DNSServiceRef sdRef_ = nullptr;

    /**
     * Called by a reply callback that wants this ref closed. The ref cannot
     * be deallocated inside DNSServiceProcessResult(), so process() closes it
     * afterwards and calls onRefError().
     */
    void failFromCallback(DNSServiceErrorType error);

    /**
     * Called after the ref was closed because DNSServiceProcessResult()
     * failed, for example when mDNSResponder restarted, or because a callback
     * called failFromCallback().
     */
    virtual void onRefError(DNSServiceErrorType error);

  private:
    DNSServiceErrorType callbackError_ = kDNSServiceErr_NoError;

    void process();
  };
} // namespace mdnscpp
