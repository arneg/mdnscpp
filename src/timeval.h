#pragma once

#include <sys/time.h>

namespace mdnscpp
{
  static inline struct timeval getNow()
  {
    struct timeval result;

    // Can not fail?
    gettimeofday(&result, nullptr);

    return result;
  }

  static inline struct timeval addTime(
      const struct timeval &a, const struct timeval &b)
  {
    struct timeval result;

    timeradd(&a, &b, &result);

    return result;
  }

  static inline struct timeval subTime(
      const struct timeval &a, const struct timeval &b)
  {
    struct timeval result;

    timersub(&a, &b, &result);

    return result;
  }

  static inline size_t timevalToMs(const struct timeval &a)
  {
    size_t result = a.tv_sec * 1000;

    return result + (a.tv_usec / 1000);
  }

  static inline bool isBefore(
      const struct timeval &lhs, const struct timeval &rhs)
  {
    int result = timercmp(&lhs, &rhs, <);
    return !!result;
  }

  static inline bool isAfter(
      const struct timeval &lhs, const struct timeval &rhs)
  {
    int result = timercmp(&lhs, &rhs, >);
    return !!result;
  }

  static inline bool isSameTime(
      const struct timeval &lhs, const struct timeval &rhs)
  {
    int result = timercmp(&lhs, &rhs, ==);
    return !!result;
  }
} // namespace mdnscpp
