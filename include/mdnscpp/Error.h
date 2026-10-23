#pragma once

#include <stdexcept>
#include <string>

namespace mdnscpp
{
  /**
   * Identifies which mDNS backend produced a PlatformError code. The integer
   * is that backend's native error (DNSServiceErrorType, Avahi, Win32
   * DNS_STATUS) and is only meaningful together with this type.
   */
  enum class PlatformType
  {
    DnsSd,
    Avahi,
    Win32,
  };

  static inline const char *describePlatformType(PlatformType type)
  {
    if (type == PlatformType::DnsSd)
    {
      return "dns_sd";
    }
    else if (type == PlatformType::Avahi)
    {
      return "avahi";
    }
    else if (type == PlatformType::Win32)
    {
      return "win32";
    }
    else
    {
      return "unknown";
    }
  }

  /**
   * Thrown if the platform's mDNS service cannot be used, e.g. no D-Bus or
   * Avahi daemon on Linux. This is an environment problem, not a programming
   * error: the same call may succeed later or on another machine.
   *
   * The same type is stored on a Browser that dies after it was created
   * (see Browser::hasError() / Browser::getError()).
   */
  class PlatformError : public std::runtime_error
  {
  public:
    PlatformError(PlatformType platform, int code, const std::string &what)
        : std::runtime_error(what), platform_(platform), code_(code)
    {
    }

    PlatformType platform() const { return platform_; }
    int code() const { return code_; }

  private:
    PlatformType platform_;
    int code_;
  };
} // namespace mdnscpp
