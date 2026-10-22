#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace mdnscpp
{
  /**
   * Converts UTF-16 to UTF-8.
   *
   * All overloads return std::nullopt if the input is not well-formed UTF-16
   * (e.g. contains an unpaired surrogate) or is too large to convert. They
   * never throw and never assert, so they are safe to use on data received
   * from the network.
   *
   * A null pointer is treated as an empty string.
   */
  std::optional<std::string> fromWideString(const wchar_t *wstr, size_t length);
  std::optional<std::string> fromWideString(const std::wstring &input);
  std::optional<std::string> fromWideString(const wchar_t *str);
  std::optional<std::string> fromWideString(const char *str);
} // namespace mdnscpp
