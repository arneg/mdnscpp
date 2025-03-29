#include "Win32Browser.h"

#include <iostream>
#include <stdexcept>

namespace mdnscpp
{
  Win32Browser::Win32Browser(std::shared_ptr<Win32Platform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : Browser(type, protocol, onResultsChanged, domain, interfaceIndex,
            ipProtocol)
  {
  }

  Win32Browser::~Win32Browser() { std::cerr << "~Win32Browser()" << std::endl; }

  std::string Win32Browser::describe() const
  {
    std::string result = "Win32Browse(";
    result += type_;
    result += ", ";
    result += protocol_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += interfaceIndex_;
    result += ")";
    return result;
  }

  std::shared_ptr<Browser> Win32Browser::getSharedFromThis()
  {
    return shared_from_this();
  }
} // namespace mdnscpp
