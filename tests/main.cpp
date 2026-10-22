#include "TestHarness.h"

namespace mdnscpp_test
{
  int runAll(const char *filter);
}

int main(int argc, char **argv)
{
  return mdnscpp_test::runAll(argc > 1 ? argv[1] : nullptr);
}
