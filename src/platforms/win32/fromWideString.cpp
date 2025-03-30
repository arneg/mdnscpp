#pragma once

#include "fromWideString.h"

#include <cwchar>

namespace mdnscpp
{
  std::string fromWideString(const std::wstring &input)
  {
    return fromWideString(input.c_str());
  }

  std::string fromWideString(const wchar_t *wstr)
  {
    std::mbstate_t state = std::mbstate_t();
    std::size_t len = 1 + std::wcsrtombs(nullptr, &wstr, 0, &state);
    std::string result;
    result.resize(len);
    std::wcsrtombs(result.data(), &wstr, len, &state);
    return result;
  }

  std::string fromWideString(const char *str)
  {
    std::string result;
    result.append(str);
    return result;
  }
} // namespace mdnscpp