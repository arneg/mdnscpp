#include "DnsSdRef.h"
#include "DnsSdPlatform.h"

#include <stdexcept>

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
    if (sdRef_)
    {
      watch_ = nullptr;
      DNSServiceRefDeallocate(sdRef_);
    }
  }

  std::shared_ptr<DnsSdPlatform> DnsSdRef::getPlatform() const
  {
    return platform_;
  }

  void DnsSdRef::process()
  {
    if (sdRef_)
    {
      DNSServiceProcessResult(sdRef_);
    }
    else
    {
      throw std::runtime_error("Calling process() on a closed ref.");
    }
  }
} // namespace mdnscpp
