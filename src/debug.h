#pragma once

#ifdef MDNSCPP_ENABLE_VERBOSE_DEBUG
#  include <iostream>
#  define MDNSCPP_INFO (std::cerr)
#  define MDNSCPP_ERROR (std::cerr << "ERROR: ")
#  define MDNSCPP_ENDL (std::endl)
#else

namespace mdnscpp
{
  struct debug_dummy
  {
    debug_dummy() = default;

    template <class... Types> debug_dummy &operator<<(Types...)
    {
      return *this;
    }
  };
} // namespace mdnscpp

#  define MDNSCPP_INFO                                                         \
    if (false)                                                                 \
    (mdnscpp::debug_dummy{})
#  define MDNSCPP_ERROR                                                        \
    if (false)                                                                 \
    (mdnscpp::debug_dummy{})

#  define MDNSCPP_ENDL (0)
#endif