#include "TestHarness.h"

#include <cstring>
#include <exception>
#include <iostream>

namespace mdnscpp_test
{
  static int currentTestFailures = 0;

  std::vector<TestCase> &registry()
  {
    // Function-local static: safe against static initialization order, as the
    // registrars run during static initialization of the test files.
    static std::vector<TestCase> tests;
    return tests;
  }

  Registrar::Registrar(const char *name, void (*function)())
  {
    registry().push_back({name, function});
  }

  void reportFailure(const char *file, int line, const std::string &message)
  {
    currentTestFailures++;
    std::cerr << "    " << file << ":" << line << ": " << message << std::endl;
  }

  // Runs all tests whose name contains the optional filter (argv[1]).
  int runAll(const char *filter)
  {
    int failedTests = 0;
    int ranTests = 0;

    for (const auto &test : registry())
    {
      if (filter && !std::strstr(test.name, filter))
        continue;

      ranTests++;
      currentTestFailures = 0;
      std::cerr << "[ RUN  ] " << test.name << std::endl;

#ifdef __cpp_exceptions
      try
      {
        test.function();
      }
      catch (const std::exception &e)
      {
        reportFailure(__FILE__, __LINE__,
            std::string("unexpected exception: ") + e.what());
      }
      catch (...)
      {
        reportFailure(__FILE__, __LINE__, "unexpected unknown exception");
      }
#else
      test.function();
#endif

      if (currentTestFailures)
      {
        failedTests++;
        std::cerr << "[ FAIL ] " << test.name << std::endl;
      }
      else
      {
        std::cerr << "[  OK  ] " << test.name << std::endl;
      }
    }

    std::cerr << ranTests << " test(s) run, " << failedTests << " failed."
              << std::endl;

    // Running nothing is a failure: it hides tests that silently dropped out
    // of the build.
    if (ranTests == 0)
    {
      std::cerr << "No tests ran." << std::endl;
      return 1;
    }

    return failedTests ? 1 : 0;
  }
} // namespace mdnscpp_test
