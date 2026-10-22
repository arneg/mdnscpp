#include "toWideString.h"

#include <windows.h>
//
#include <stringapiset.h>

#include <limits>

namespace mdnscpp
{
  std::optional<std::wstring> toWideString(const std::string &input)
  {
    if (input.empty())
      return std::wstring();

    // The Win32 API takes an int length. Note: (max)() is parenthesized so
    // that it is not expanded by the max macro from windows.h.
    if (input.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
      return std::nullopt;

    const int inputLength = static_cast<int>(input.size());

    // MB_ERR_INVALID_CHARS makes the conversion fail on malformed input.
    // Without it, Windows silently substitutes or drops invalid sequences,
    // which would hide that the data was damaged or malicious.
    int count = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), inputLength, NULL, 0);

    if (count <= 0)
      return std::nullopt;

    std::wstring result(static_cast<size_t>(count), L'\0');

    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
            inputLength, result.data(), count) <= 0)
      return std::nullopt;

    return result;
  }
} // namespace mdnscpp
