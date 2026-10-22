#pragma once

// A deliberately tiny, dependency-free test harness.
//
//   TEST_CASE(name_of_test) { CHECK(1 + 1 == 2); }
//
// CHECK() records a failure and continues, so one test reports all of its
// failing expectations. The process exit code is non-zero if anything failed.

#include <string>
#include <vector>

namespace mdnscpp_test
{
  struct TestCase
  {
    const char *name;
    void (*function)();
  };

  std::vector<TestCase> &registry();

  struct Registrar
  {
    Registrar(const char *name, void (*function)());
  };

  void reportFailure(const char *file, int line, const std::string &message);
} // namespace mdnscpp_test

#define TEST_CASE(NAME)                                                        \
  static void NAME();                                                          \
  static mdnscpp_test::Registrar registrar_##NAME(#NAME, NAME);                \
  static void NAME()

#define CHECK(EXPR)                                                            \
  do                                                                           \
  {                                                                            \
    if (!(EXPR))                                                               \
      mdnscpp_test::reportFailure(__FILE__, __LINE__, "CHECK(" #EXPR ")");     \
  } while (0)
