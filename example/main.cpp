#include <mdnscpp/Platform.h>
#include <mdnscpp/PollLoop.h>
#include <mdnscpp/utils.h>

#include <iostream>

int main(int argc, const char **argv)
{
  mdnscpp::PollLoop loop;

  auto platform = mdnscpp::createPlatform(loop);

  auto browser = platform->createBrowser("_oca", "_tcp", [](auto browser) {
    auto results = mdnscpp::getSortedList(browser->getResults());
    std::cout << "Results (" << results.size() << "): " << std::endl;
    for (const auto &result : results)
    {
      std::cout << result.describe() << std::endl;
    }
  });

  loop.run();

  return 0;
}
