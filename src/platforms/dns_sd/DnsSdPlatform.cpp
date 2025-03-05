#include "DnsSdPlatform.h"
#include "DnsSdBrowser.h"
#include "DnsSdRef.h"

#include <dns_sd.h>

#include <iostream>

namespace mdnscpp
{
  DnsSdPlatform::DnsSdPlatform(EventLoop &loop) : Platform(loop) {}

  std::shared_ptr<Browser> DnsSdPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interface, IPProtocol ipProtocol)
  {
    return std::make_shared<DnsSdBrowser>(shared_from_this(), type, protocol,
        onResultsChanged, domain, interface, ipProtocol);
  }
} // namespace mdnscpp
