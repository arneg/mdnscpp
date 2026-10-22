// Tests for the UTF-8 <-> UTF-16 conversions used on data received from the
// network. The contract being tested: malformed input is *reported* (nullopt),
// never silently repaired, never thrown and never asserted on.
//
// Raw bytes are written as octal escapes. Hex escapes are greedy and would
// swallow following characters that happen to be hex digits.

#include "../TestHarness.h"

#include "platforms/win32/fromWideString.h"
#include "platforms/win32/toWideString.h"

#include <limits>

using namespace mdnscpp;

namespace
{
  std::wstring wide(std::initializer_list<unsigned int> units)
  {
    std::wstring result;
    for (unsigned int unit : units)
      result.push_back(static_cast<wchar_t>(unit));
    return result;
  }
} // namespace

// ---------------------------------------------------------------------------
// toWideString: UTF-8 -> UTF-16
// ---------------------------------------------------------------------------

TEST_CASE(toWideString_ascii)
{
  auto result = toWideString("hello");
  CHECK(result.has_value());
  CHECK(result && *result == L"hello");
}

TEST_CASE(toWideString_empty)
{
  auto result = toWideString("");
  CHECK(result.has_value());
  CHECK(result && result->empty());
}

TEST_CASE(toWideString_two_and_three_byte_sequences)
{
  // U+00E9 (e acute), U+20AC (euro sign)
  auto result = toWideString("\303\251\342\202\254");
  CHECK(result.has_value());
  CHECK(result && *result == wide({0x00E9, 0x20AC}));
}

TEST_CASE(toWideString_non_bmp_becomes_surrogate_pair)
{
  // U+1F600
  auto result = toWideString("\360\237\230\200");
  CHECK(result.has_value());
  CHECK(result && *result == wide({0xD83D, 0xDE00}));
}

TEST_CASE(toWideString_keeps_embedded_nul)
{
  auto result = toWideString(std::string("a\0b", 3));
  CHECK(result.has_value());
  CHECK(result && result->size() == 3);
  CHECK(result && *result == wide({'a', 0, 'b'}));
}

TEST_CASE(toWideString_rejects_lone_continuation_byte)
{
  CHECK(!toWideString("a\200b"));
}

TEST_CASE(toWideString_rejects_truncated_sequence)
{
  // First two bytes of a three byte sequence, at the end and in the middle.
  CHECK(!toWideString("\342\202"));
  CHECK(!toWideString("a\342\202"
                      "b"));
}

TEST_CASE(toWideString_rejects_invalid_lead_byte)
{
  CHECK(!toWideString("\377"));
  CHECK(!toWideString("\376"));
}

TEST_CASE(toWideString_rejects_overlong_encoding)
{
  // Overlong encoding of U+0000 and of '/'.
  CHECK(!toWideString("\300\200"));
  CHECK(!toWideString("\300\257"));
}

TEST_CASE(toWideString_rejects_encoded_surrogate)
{
  // U+D800 encoded as if it were a regular code point (CESU-8 style).
  CHECK(!toWideString("\355\240\200"));
}

TEST_CASE(toWideString_rejects_code_point_above_unicode_range)
{
  // U+110000
  CHECK(!toWideString("\364\220\200\200"));
}

// ---------------------------------------------------------------------------
// fromWideString: UTF-16 -> UTF-8
// ---------------------------------------------------------------------------

TEST_CASE(fromWideString_ascii)
{
  auto result = fromWideString(std::wstring(L"hello"));
  CHECK(result.has_value());
  CHECK(result && *result == "hello");
}

TEST_CASE(fromWideString_empty)
{
  auto result = fromWideString(std::wstring());
  CHECK(result.has_value());
  CHECK(result && result->empty());
}

TEST_CASE(fromWideString_bmp_characters)
{
  auto result = fromWideString(wide({0x00E9, 0x20AC}));
  CHECK(result.has_value());
  CHECK(result && *result == "\303\251\342\202\254");
}

TEST_CASE(fromWideString_surrogate_pair_becomes_four_bytes)
{
  auto result = fromWideString(wide({0xD83D, 0xDE00}));
  CHECK(result.has_value());
  CHECK(result && *result == "\360\237\230\200");
}

TEST_CASE(fromWideString_keeps_embedded_nul_with_explicit_length)
{
  const wchar_t input[] = {L'a', 0, L'b'};
  auto result = fromWideString(input, 3);
  CHECK(result.has_value());
  CHECK(result && *result == std::string("a\0b", 3));
}

TEST_CASE(fromWideString_null_terminated_stops_at_nul)
{
  const wchar_t input[] = {L'a', L'b', 0, L'c', 0};
  auto result = fromWideString(static_cast<const wchar_t *>(input));
  CHECK(result.has_value());
  CHECK(result && *result == "ab");
}

TEST_CASE(fromWideString_null_pointer_is_empty_string)
{
  // E.g. a TXT record without value.
  auto terminated = fromWideString(static_cast<const wchar_t *>(nullptr));
  CHECK(terminated.has_value());
  CHECK(terminated && terminated->empty());

  auto counted = fromWideString(static_cast<const wchar_t *>(nullptr), 5);
  CHECK(counted.has_value());
  CHECK(counted && counted->empty());

  auto narrow = fromWideString(static_cast<const char *>(nullptr));
  CHECK(narrow.has_value());
  CHECK(narrow && narrow->empty());
}

TEST_CASE(fromWideString_zero_length_does_not_read_input)
{
  // The pointer must not be dereferenced for length 0.
  auto result = fromWideString(reinterpret_cast<const wchar_t *>(1), 0);
  CHECK(result.has_value());
  CHECK(result && result->empty());
}

TEST_CASE(fromWideString_rejects_unpaired_high_surrogate)
{
  CHECK(!fromWideString(wide({'a', 0xD800, 'b'})));
  // At the end of the input, nothing follows.
  CHECK(!fromWideString(wide({'a', 0xD800})));
  // Followed by another high surrogate.
  CHECK(!fromWideString(wide({0xD800, 0xD800, 0xDC00})));
}

TEST_CASE(fromWideString_rejects_unpaired_low_surrogate)
{
  CHECK(!fromWideString(wide({'a', 0xDC00, 'b'})));
  CHECK(!fromWideString(wide({0xDC00})));
}

TEST_CASE(fromWideString_rejects_reversed_surrogate_pair)
{
  CHECK(!fromWideString(wide({0xDE00, 0xD83D})));
}

TEST_CASE(fromWideString_rejects_length_that_does_not_fit_in_int)
{
  // Must be rejected without reading from the pointer. A truncated length
  // would become negative; -1 would even mean "null terminated" to Win32.
  if (sizeof(size_t) > sizeof(int))
  {
    const wchar_t input[] = L"x";
    const size_t tooLong =
        static_cast<size_t>((std::numeric_limits<int>::max)()) + 1;
    CHECK(!fromWideString(input, tooLong));

    const size_t lookslikeMinusOne = 0xFFFFFFFFu;
    CHECK(!fromWideString(input, lookslikeMinusOne));
  }
}

// ---------------------------------------------------------------------------
// Round trips
// ---------------------------------------------------------------------------

TEST_CASE(round_trip_utf8_wide_utf8)
{
  const std::string original = "caf\303\251 \342\202\254 \360\237\230\200";

  auto asWide = toWideString(original);
  CHECK(asWide.has_value());
  if (!asWide)
    return;

  auto back = fromWideString(*asWide);
  CHECK(back.has_value());
  CHECK(back && *back == original);
}
