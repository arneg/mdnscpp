#include "Win32Platform.h"
#include "Win32Browser.h"

#include <iostream>

namespace mdnscpp
{
  Win32Platform::Win32Platform(EventLoop &loop) : Platform(loop) {}

  std::shared_ptr<Browser> Win32Platform::createBrowser(const std::string &type,
      const std::string &protocol,
      Browser::ResultsChangedCallback onResultsChanged,
      const std::string &domain, size_t interfaceIndex, IPProtocol ipProtocol)
  {
    return std::make_shared<Win32Browser>(shared_from_this(), type, protocol,
        onResultsChanged, domain, interfaceIndex, ipProtocol);
  }
} // namespace mdnscpp
