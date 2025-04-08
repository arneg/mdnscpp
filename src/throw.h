#pragma once

#include <cassert>

#ifdef __cpp_exceptions
#  include <stdexcept>
#elif !defined(NDEBUG)
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

#define MDNSCPP_ASSERT(X) assert(X)

#ifdef NDEBUG
#  define MDNSCPP_DEBUG_ASSERT(X) assert(X)
#else
#  define MDNSCPP_DEBUG_ASSERT(X)
#endif