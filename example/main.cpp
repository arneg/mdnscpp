#include <mdnscpp/Platform.h>
#include <mdnscpp/PollLoop.h>

#include <iostream>

int main(int argc, const char **argv)
{
  mdnscpp::PollLoop loop;

  auto platform = mdnscpp::createPlatform(loop);

  auto browser = platform->createBrowser("_oca", "_tcp", nullptr);

  loop.run();

  return 0;
}
