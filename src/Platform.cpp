#include <mdnscpp/EventLoop.h>
#include <mdnscpp/Platform.h>

#include <stdexcept>

#ifdef LIBMDNS_PLATFORM_AVAHI
#  include "platforms/avahi/AvahiPlatform.h"
#elif defined(LIBMDNS_PLATFORM_DNSSD)
#  include "platforms/DnsSd/DnsSdPlatform.h"
#else
#  error "No mdns platform library found."
#endif

namespace mdnscpp
{
  Platform::Platform(EventLoop &loop) : loop_(loop) {}

  EventLoop &Platform::getEventLoop() const { return loop_; }

  std::shared_ptr<Platform> createPlatform(EventLoop &loop)
  {
#ifdef LIBMDNS_PLATFORM_AVAHI
    return std::make_shared<AvahiPlatform>(loop);
#elif defined(LIBMDNS_PLATFORM_DNSSD)
    return std::make_shared<DnsSdPlatform>(loop);
#else
#endif
  }
} // namespace mdnscpp
