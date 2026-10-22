#include "fromWideString.h"

#include <windows.h>
//
#include <stringapiset.h>

#include <cwchar>
#include <limits>

namespace mdnscpp
{
  std::optional<std::string> fromWideString(const wchar_t *wstr, size_t length)
  {
    if (!wstr || length == 0)
      return std::string();

    // The Win32 API takes an int length. Do not truncate, a negative value
    // (-1 in particular) would be interpreted as "null terminated".
    // Note: (max)() is parenthesized so that it is not expanded by the max
    // macro from windows.h.
    if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
      return std::nullopt;

    const int inputLength = static_cast<int>(length);

    // WC_ERR_INVALID_CHARS makes the conversion fail on malformed input
    // (unpaired surrogates). Without it, Windows silently substitutes
    // U+FFFD, which would hide that the data was damaged or malicious.
    int count = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, wstr, inputLength, NULL, 0, NULL, NULL);

    if (count <= 0)
      return std::nullopt;

    std::string result(static_cast<size_t>(count), '\0');

    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wstr, inputLength,
            result.data(), count, NULL, NULL) <= 0)
      return std::nullopt;

    return result;
  }

  std::optional<std::string> fromWideString(const std::wstring &input)
  {
    return fromWideString(input.data(), input.size());
  }

  std::optional<std::string> fromWideString(const wchar_t *wstr)
  {
    if (!wstr)
      return std::string();
    return fromWideString(wstr, std::wcslen(wstr));
  }

  std::optional<std::string> fromWideString(const char *str)
  {
    if (!str)
      return std::string();
    return std::string(str);
  }
} // namespace mdnscpp
