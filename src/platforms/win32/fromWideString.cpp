#pragma once

#include "fromWideString.h"
#include "../../throw.h"

#include <windows.h>
//
#include <stringapiset.h>

#include <cwchar>

namespace mdnscpp
{
  std::string fromWideString(const wchar_t *wstr, size_t length)
  {
    std::string result;
    if (length)
    {
      int count = WideCharToMultiByte(
          CP_UTF8, 0, wstr, static_cast<int>(length), NULL, 0, NULL, NULL);
      if (count <= 0)
        MDNSCPP_THROW(std::invalid_argument, "Invalid wide character input.");
      result.resize(count);
      if (WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(length),
              result.data(), count, NULL, NULL) <= 0)
        MDNSCPP_THROW(std::invalid_argument, "Invalid wide character input.");
    }
    return result;
  }

  std::string fromWideString(const std::wstring &input)
  {
    return fromWideString(input.data(), input.size());
  }

  std::string fromWideString(const wchar_t *wstr)
  {
    return fromWideString(wstr, std::wcslen(wstr));
  }

  std::string fromWideString(const char *str)
  {
    std::string result;
    result.append(str);
    return result;
  }
} // namespace mdnscpp