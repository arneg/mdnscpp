#pragma once

// Safety net for C-to-C++ boundaries. Catches any exception thrown from a
// callback, logs it and returns, so that it cannot unwind through C frames
// (libuv, Avahi, dns_sd, Win32). When exceptions are compiled out this is
// a pass-through: MDNSCPP_THROW already aborts.
//
// Must be used at the innermost trampoline. Catching only in the event loop
// is too late if Avahi or dns_sd is on the stack.

#include "debug.h"

#include <utility>

#ifdef __cpp_exceptions
#  include <exception>
#endif

namespace mdnscpp
{
#ifdef __cpp_exceptions
  template <class Fn> void guardCallback(const char *where, Fn &&fn)
  {
    try
    {
      std::forward<Fn>(fn)();
    }
    catch (const std::exception &e)
    {
      MDNSCPP_ERROR << where << " threw: " << e.what() << MDNSCPP_ENDL;
    }
    catch (...)
    {
      MDNSCPP_ERROR << where << " threw a non-std exception." << MDNSCPP_ENDL;
    }
  }

  template <class Fn, class Fallback>
  auto guardCallback(const char *where, Fn &&fn, Fallback fallback)
      -> decltype(std::forward<Fn>(fn)())
  {
    try
    {
      return std::forward<Fn>(fn)();
    }
    catch (const std::exception &e)
    {
      MDNSCPP_ERROR << where << " threw: " << e.what() << MDNSCPP_ENDL;
      return fallback;
    }
    catch (...)
    {
      MDNSCPP_ERROR << where << " threw a non-std exception." << MDNSCPP_ENDL;
      return fallback;
    }
  }
#else
  template <class Fn> void guardCallback(const char *, Fn &&fn)
  {
    std::forward<Fn>(fn)();
  }

  template <class Fn, class Fallback>
  auto guardCallback(const char *, Fn &&fn, Fallback)
      -> decltype(std::forward<Fn>(fn)())
  {
    return std::forward<Fn>(fn)();
  }
#endif
} // namespace mdnscpp
