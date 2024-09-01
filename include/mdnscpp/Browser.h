#pragma once

#include <functional>
#include <string>

namespace mdnscpp
{
  class Browser
  {
  public:
    Browser(const std::string &type, const std::string &protocol,
        std::function<void(const Browser &)> onResultsChanged,
        const std::string &domain, size_t interface);
    virtual ~Browser() = default;

    const std::string &getType() const;
    const std::string &getProtocol() const;
    const std::string &getDomain() const;
    size_t getInterface() const;

  protected:
    const std::string type_;
    const std::string protocol_;
    std::function<void(const Browser &)> onResultsChanged_;
    const std::string domain_;
    const size_t interface_;
  };
} // namespace mdnscpp
