#include "DnsSdRef.h"
#include "DnsSdPlatform.h"

#include "../../debug.h"

namespace mdnscpp
{
  DnsSdRef::DnsSdRef(
      std::shared_ptr<DnsSdPlatform> platform, DNSServiceRef sdRef)
      : platform_(platform), sdRef_(sdRef)
  {
    if (sdRef_)
    {
      int fd = DNSServiceRefSockFD(sdRef_);
      watch_ = platform->getEventLoop().createWatch(fd,
          EventLoop::EventType::TYPE_READ,
          [this](EventLoop::EventType) { process(); });
    }
  }

  DnsSdRef::~DnsSdRef() { close(); }

  void DnsSdRef::close()
  {
    watch_ = nullptr;
    if (sdRef_)
    {
      DNSServiceRefDeallocate(sdRef_);
      sdRef_ = nullptr;
    }
  }

  std::shared_ptr<DnsSdPlatform> DnsSdRef::getPlatform() const
  {
    return platform_;
  }

  void DnsSdRef::failFromCallback(DNSServiceErrorType error)
  {
    callbackError_ = error;
  }

  void DnsSdRef::onRefError(DNSServiceErrorType) {}

  void DnsSdRef::process()
  {
    if (!sdRef_)
      return;

    DNSServiceErrorType error = DNSServiceProcessResult(sdRef_);

    if (error == kDNSServiceErr_NoError)
      error = callbackError_;

    if (error != kDNSServiceErr_NoError)
    {
      MDNSCPP_ERROR << "DNSServiceProcessResult() failed with " << error
                    << MDNSCPP_ENDL;
      callbackError_ = kDNSServiceErr_NoError;
      // Without this the socket stays readable (POLLHUP) and the loop calls
      // process() without end.
      close();
      onRefError(error);
    }
  }
} // namespace mdnscpp
