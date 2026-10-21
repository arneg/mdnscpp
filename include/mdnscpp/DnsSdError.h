#pragma once

#include <stdexcept>
#include <string>

namespace mdnscpp
{
  /**
   * Thrown when a dns_sd call fails. code() is the DNSServiceErrorType, for
   * example -65570 (kDNSServiceErr_PolicyDenied) when Local Network access
   * is denied.
   */
  class DnsSdError : public std::runtime_error
  {
  public:
    DnsSdError(const std::string &what, int code)
        : std::runtime_error(what + " (" + std::to_string(code) + ")"),
          code_(code)
    {
    }

    int code() const { return code_; }

  private:
    int code_;
  };
} // namespace mdnscpp
