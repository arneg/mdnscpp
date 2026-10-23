#pragma once

#include <cassert>

#ifdef __cpp_exceptions
#  include <stdexcept>
#elif !defined(NDEBUG)
#  include <stdexcept>
#else
#  include <exception>
#endif

#ifdef __cpp_exceptions
#  define MDNSCPP_THROW(EXCEPTION_TYPE, ...)                                   \
    do                                                                         \
    {                                                                          \
      throw EXCEPTION_TYPE(__VA_ARGS__);                                       \
    } while (0);
#elif !defined(NDEBUG)
#  define MDNSCPP_THROW(EXCEPTION_TYPE, ...)                                   \
    do                                                                         \
    {                                                                          \
      /* Arguments may be any expressions convertible to the exception's     \
         constructor, e.g. a std::string, so they cannot be negated. */ \
      assert(((void)EXCEPTION_TYPE(__VA_ARGS__), false));                      \
    } while (0);
#else
#  define MDNSCPP_THROW(EXCEPTION_TYPE, ...)                                   \
    do                                                                         \
    {                                                                          \
      std::terminate();                                                        \
    } while (0);
#endif

#define MDNSCPP_ASSERT(X) assert(X)

#ifndef NDEBUG
#  define MDNSCPP_DEBUG_ASSERT(X) assert(X)
#else
#  define MDNSCPP_DEBUG_ASSERT(X)
#endif
