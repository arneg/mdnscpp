#include "toWideString.h"

#include <cwchar>

namespace mdnscpp
{
  std::wstring toWideString(const std::string &input)
  {
    std::wstring result;
    const char *src = input.data();
    std::mbstate_t state = std::mbstate_t();
    std::size_t len = 1 + std::mbsrtowcs(nullptr, &src, 0, &state);
    result.resize(len);
    std::mbsrtowcs(&result[0], &src, len, &state);

    return result;
  }
} // namespace mdnscpp