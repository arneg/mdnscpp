#pragma once

#include <stdexcept>

namespace mdnscpp
{
  /**
   * Thrown if the platform's mDNS service cannot be used, e.g. no D-Bus or
   * Avahi daemon on Linux. This is an environment problem, not a programming
   * error: the same call may succeed later or on another machine.
   */
  class PlatformError : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
} // namespace mdnscpp
