#include <mdnscpp/CallQueue.h>
#include <mdnscpp/DefaultLoop.h>
#include <mdnscpp/Platform.h>
#include <mdnscpp/utils.h>

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

int main(int argc, const char **argv)
{
  mdnscpp::DefaultLoop loop;

  auto platform = mdnscpp::createPlatform(loop);

  auto browser = platform->createBrowser("_oca", "_tcp", [](auto browser) {
    auto results = mdnscpp::getSortedList(browser->getResults());
    std::cout << "Results (" << results.size() << "): " << std::endl;
    for (const auto &result : results)
    {
      std::cout << result.describe() << std::endl;
    }
  });

  auto callQueue = mdnscpp::CallQueue::create(loop);

  std::thread thread{[callQueue]() {
    while (true)
    {
      using namespace std::chrono_literals;

      std::promise<void> p;
      callQueue->schedule([&]() { p.set_value(); });
      std::cerr << "waiting for callback" << std::endl;
      p.get_future().wait();
      std::this_thread::sleep_for(50s);
    }
  }};

  loop.run();

  return 0;
}
