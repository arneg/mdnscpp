#include "TestHarness.h"

#include <mdnscpp/validation.h>

#include <algorithm>
#include <string>

using namespace mdnscpp;
using Argument = BrowseArgumentError::Argument;

namespace
{
  // Valid defaults, each test varies one argument.
  bool valid(const std::string &type, const std::string &protocol = "_tcp",
      const std::string &domain = "", size_t interfaceIndex = 0)
  {
    return !validateBrowseArguments(type, protocol, domain, interfaceIndex);
  }

  // Returns true if validation fails and blames the expected argument.
  bool rejects(Argument expected, const std::string &type,
      const std::string &protocol = "_tcp", const std::string &domain = "",
      size_t interfaceIndex = 0)
  {
    auto error =
        validateBrowseArguments(type, protocol, domain, interfaceIndex);
    return error && error->argument == expected && !error->message.empty();
  }

  std::string repeat(char c, size_t count) { return std::string(count, c); }
} // namespace

// ---------------------------------------------------------------------------
// type
// ---------------------------------------------------------------------------

TEST_CASE(validation_accepts_typical_types)
{
  CHECK(valid("_http"));
  CHECK(valid("_ttest"));
  CHECK(valid("_airplay-bt"));
  CHECK(valid("_A1"));
}

TEST_CASE(validation_type_length_limits)
{
  CHECK(valid("_" + repeat('a', 1)));
  CHECK(valid("_" + repeat('a', 15)));
  CHECK(rejects(Argument::Type, "_" + repeat('a', 16)));
  CHECK(rejects(Argument::Type, "_"));
  CHECK(rejects(Argument::Type, ""));
}

TEST_CASE(validation_type_requires_leading_underscore)
{
  CHECK(rejects(Argument::Type, "http"));
  CHECK(rejects(Argument::Type, "-http"));
}

TEST_CASE(validation_type_rejects_invalid_characters)
{
  CHECK(rejects(Argument::Type, "_ht tp"));
  CHECK(rejects(Argument::Type, "_http."));
  CHECK(rejects(Argument::Type, "_http._tcp"));
  CHECK(rejects(Argument::Type, "_a_b"));
  CHECK(rejects(Argument::Type, std::string("_a\0b", 4)));
  CHECK(rejects(Argument::Type, "_caf\303\251"));
}

TEST_CASE(validation_type_rejects_subtypes)
{
  CHECK(rejects(Argument::Type, "_printer._sub._http"));
}

// ---------------------------------------------------------------------------
// protocol
// ---------------------------------------------------------------------------

TEST_CASE(validation_protocol_accepts_tcp_and_udp)
{
  CHECK(valid("_http", "_tcp"));
  CHECK(valid("_http", "_udp"));
}

TEST_CASE(validation_protocol_rejects_everything_else)
{
  CHECK(rejects(Argument::Protocol, "_http", ""));
  CHECK(rejects(Argument::Protocol, "_http", "tcp"));
  CHECK(rejects(Argument::Protocol, "_http", "_TCP"));
  CHECK(rejects(Argument::Protocol, "_http", "_sctp"));
  CHECK(rejects(Argument::Protocol, "_http", "_tcp "));
  CHECK(rejects(Argument::Protocol, "_http", std::string("_tcp\0", 5)));
}

// ---------------------------------------------------------------------------
// domain
// ---------------------------------------------------------------------------

TEST_CASE(validation_domain_accepts_empty_and_regular_names)
{
  CHECK(valid("_http", "_tcp", ""));
  CHECK(valid("_http", "_tcp", "local"));
  CHECK(valid("_http", "_tcp", "local."));
  CHECK(valid("_http", "_tcp", "example.com"));
  CHECK(valid("_http", "_tcp", "a"));
}

TEST_CASE(validation_domain_accepts_valid_utf8)
{
  CHECK(valid("_http", "_tcp", "caf\303\251.example"));
  CHECK(valid("_http", "_tcp", "\360\237\230\200.example"));
}

TEST_CASE(validation_domain_rejects_invalid_utf8)
{
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a\200b"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a\342\202"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "\300\200"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "\355\240\200"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "\364\220\200\200"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "\377"));
}

TEST_CASE(validation_domain_rejects_control_characters_and_backslash)
{
  CHECK(rejects(Argument::Domain, "_http", "_tcp", std::string("a\0b", 3)));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a\nb"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a\177b"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a\\.b"));
}

TEST_CASE(validation_domain_rejects_empty_labels)
{
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "."));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", ".."));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a..b"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", ".a"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "a.b.."));
}

TEST_CASE(validation_domain_label_length_limit)
{
  CHECK(valid("_http", "_tcp", repeat('a', 63)));
  CHECK(valid("_http", "_tcp", repeat('a', 63) + ".local"));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", repeat('a', 64)));
  CHECK(rejects(Argument::Domain, "_http", "_tcp", "local." + repeat('a', 64)));
}

TEST_CASE(validation_domain_length_counts_bytes_not_characters)
{
  // 32 two byte characters make a 64 byte label.
  std::string label;
  for (int i = 0; i < 32; i++)
    label += "\303\251";
  CHECK(rejects(Argument::Domain, "_http", "_tcp", label));
}

TEST_CASE(validation_domain_total_length_limit)
{
  // The queried name is <type>.<protocol>.<domain> and may be at most 253
  // bytes. With "_t" and "_tcp" the prefix "_t._tcp." takes 8 bytes, so the
  // domain may be at most 245 bytes. Each domain below is individually valid
  // (<= 253 bytes, labels <= 63 bytes).
  const std::string type = "_t";
  const std::string protocol = "_tcp";

  auto makeDomain = [](size_t length) {
    // Labels of 60 bytes separated by dots, the last label takes the rest.
    std::string result;
    while (result.size() < length)
    {
      if (!result.empty())
        result += '.';
      result += repeat('a', std::min<size_t>(60, length - result.size()));
    }
    return result.substr(0, length);
  };

  CHECK(valid(type, protocol, makeDomain(245)));
  CHECK(rejects(Argument::Domain, type, protocol, makeDomain(246)));
}

TEST_CASE(validation_domain_alone_longer_than_253_is_rejected)
{
  std::string domain;
  for (int i = 0; i < 5; i++)
  {
    if (i)
      domain += '.';
    domain += repeat('a', 60);
  }
  // 5 * 60 + 4 = 304 bytes.
  CHECK(rejects(Argument::Domain, "_http", "_tcp", domain));
}

// ---------------------------------------------------------------------------
// interfaceIndex
// ---------------------------------------------------------------------------

TEST_CASE(validation_interface_index_range)
{
  CHECK(valid("_http", "_tcp", "", 0));
  CHECK(valid("_http", "_tcp", "", 1));
  CHECK(valid("_http", "_tcp", "", 2147483647u));
  CHECK(rejects(Argument::InterfaceIndex, "_http", "_tcp", "", 2147483648u));
  CHECK(rejects(Argument::InterfaceIndex, "_http", "_tcp", "", 4294967295u));
  CHECK(rejects(
      Argument::InterfaceIndex, "_http", "_tcp", "", static_cast<size_t>(-1)));
}

// ---------------------------------------------------------------------------
// ordering
// ---------------------------------------------------------------------------

TEST_CASE(validation_reports_first_invalid_argument)
{
  auto error = validateBrowseArguments("bad", "bad", "a..b", 0xFFFFFFFFu);
  CHECK(error.has_value());
  CHECK(error && error->argument == Argument::Type);

  error = validateBrowseArguments("_http", "bad", "a..b", 0xFFFFFFFFu);
  CHECK(error && error->argument == Argument::Protocol);

  error = validateBrowseArguments("_http", "_tcp", "a..b", 0xFFFFFFFFu);
  CHECK(error && error->argument == Argument::Domain);

  error = validateBrowseArguments("_http", "_tcp", "local", 0xFFFFFFFFu);
  CHECK(error && error->argument == Argument::InterfaceIndex);
}
