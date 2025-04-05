#pragma once

#ifdef __cpp_exceptions
#  include <stdexcept>
#elif !defined(NDEBUG)
#  include <cassert>
#else
#  include <exception>
#endif

#ifdef __cpp_exceptions
#  define MDNSCPP_THROW(EXCEPTION_TYPE, MSG)                                   \
    do                                                                         \
    {                                                                          \
      throw EXCEPTION_TYPE(MSG);                                               \
    } while (0);
#elif !defined(NDEBUG)
#  define MDNSCPP_THROW(EXCEPTION_TYPE, MSG)                                   \
    do                                                                         \
    {                                                                          \
      assert(!(MSG));                                                          \
    } while (0);
#else
#  define MDNSCPP_THROW(EXCEPTION_TYPE, MSG)                                   \
    do                                                                         \
    {                                                                          \
      std::terminate();                                                        \
    } while (0);
#endif