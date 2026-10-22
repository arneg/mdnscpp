#include "TestHarness.h"

#include <mdnscpp/BrowseResult.h>

using namespace mdnscpp;

namespace
{
  BrowseResult makeResult(const std::string &name = "example",
      const std::string &domain = "local",
      const std::string &address = "192.168.1.2")
  {
    return BrowseResult({}, "_http", "_tcp", name, domain, "host.local",
        address, 0, IPProtocol::IPv4, 80);
  }
} // namespace

TEST_CASE(browseResult_fullname_includes_all_parts)
{
  CHECK(makeResult().getFullname() == "example._http._tcp.local");
  CHECK(makeResult("other", "example.com").getFullname() ==
        "other._http._tcp.example.com");
}

// The constructor takes its arguments by value and moves them into members;
// the domain used to be read after being moved from.
TEST_CASE(browseResult_fullname_keeps_domain_when_moved_in)
{
  std::string domain = "local";
  BrowseResult result({}, "_http", "_tcp", "example", std::move(domain),
      "host.local", "192.168.1.2", 0, IPProtocol::IPv4, 80);

  CHECK(result.getDomain() == "local");
  CHECK(result.getFullname() == "example._http._tcp.local");
}

TEST_CASE(browseResult_ordering_distinguishes_domains)
{
  const auto a = makeResult("example", "a.example");
  const auto b = makeResult("example", "b.example");

  // Exactly one of the two orderings holds.
  CHECK((a < b) != (b < a));
  CHECK(a < b);
}

TEST_CASE(browseResult_ordering_is_strict_for_equal_results)
{
  const auto a = makeResult();
  const auto b = makeResult();

  CHECK(!(a < b));
  CHECK(!(b < a));
  CHECK(a == b);
}

TEST_CASE(browseResult_equality_compares_addresses)
{
  CHECK(makeResult("example", "local", "10.0.0.1") !=
        makeResult("example", "local", "10.0.0.2"));
}
