#pragma once

#include <optional>
#include <string>

namespace mdnscpp
{
  /**
   * Converts a UTF-8 string to UTF-16.
   *
   * Returns std::nullopt if the input is not well-formed UTF-8 (or too large
   * to convert). Never throws and never asserts, so it is safe to use on
   * data received from the network.
   */
  std::optional<std::wstring> toWideString(const std::string &input);
} // namespace mdnscpp
