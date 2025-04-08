#include "DnsSdPlatform.h"
#include "DnsSdBrowser.h"
#include "DnsSdRef.h"

#include <dns_sd.h>

#include "../../debug.h"

namespace mdnscpp
{
  DnsSdPlatform::DnsSdPlatform(EventLoop &loop) : Platform(loop) {}

  std::shared_ptr<Browser> DnsSdPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      Browser::ResultsChangedCallback onResultsChanged,
      const std::string &domain, size_t interfaceIndex, IPProtocol ipProtocol)
  {
    return std::make_shared<DnsSdBrowser>(shared_from_this(), type, protocol,
        onResultsChanged, domain, interfaceIndex, ipProtocol);
  }
} // namespace mdnscpp
