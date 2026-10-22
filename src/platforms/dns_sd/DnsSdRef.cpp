#include "DnsSdRef.h"
#include "DnsSdPlatform.h"

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

  void DnsSdRef::process()
  {
    if (sdRef_)
      DNSServiceProcessResult(sdRef_);
  }
} // namespace mdnscpp
