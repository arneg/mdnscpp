#pragma once

#include <functional>
#include <string>

#include "Types.h"

namespace mdnscpp
{
  class Browser
  {
  public:
    Browser(const std::string &type, const std::string &protocol,
        std::function<void(const Browser &)> onResultsChanged,
        const std::string &domain, size_t interface, IPProtocol ipProtocol);
    virtual ~Browser() = default;

    const std::string &getType() const;
    const std::string &getProtocol() const;
    const std::string &getDomain() const;
    size_t getInterface() const;
    IPProtocol getIPProtocol() const;

  protected:
    const std::string type_;
    const std::string protocol_;
    const std::function<void(const Browser &)> onResultsChanged_;
    const std::string domain_;
    const size_t interface_;
    const IPProtocol ipProtocol_;
  };
} // namespace mdnscpp
