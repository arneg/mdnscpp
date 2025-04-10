#pragma once

#include <string>

namespace mdnscpp
{
  std::string fromWideString(const wchar_t *wstr, size_t length);
  std::string fromWideString(const std::wstring &input);
  std::string fromWideString(const wchar_t *str);
  std::string fromWideString(const char *str);
} // namespace mdnscpp