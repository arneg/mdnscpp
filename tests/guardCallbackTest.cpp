#include "TestHarness.h"

#include "guardCallback.h"

#ifdef __cpp_exceptions
#  include <stdexcept>
#endif

using namespace mdnscpp;

TEST_CASE(guardCallback_runs_the_function)
{
  int n = 0;
  guardCallback("test", [&] { n = 1; });
  CHECK(n == 1);
}

#ifdef __cpp_exceptions
TEST_CASE(guardCallback_swallows_std_exception)
{
  bool ran = false;
  guardCallback("test", [&] {
    ran = true;
    throw std::runtime_error("boom");
  });
  CHECK(ran);
}

TEST_CASE(guardCallback_swallows_unknown_exception)
{
  guardCallback("test", [&] { throw 42; });
}

TEST_CASE(guardCallback_returns_fallback_on_throw)
{
  const int value = guardCallback(
      "test",
      [&]() -> int {
        throw std::runtime_error("boom");
        return 1;
      },
      7);
  CHECK(value == 7);
}

TEST_CASE(guardCallback_returns_the_function_result)
{
  const int value = guardCallback("test", [&]() -> int { return 3; }, 7);
  CHECK(value == 3);
}
#endif
