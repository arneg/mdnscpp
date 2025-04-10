#include "toWideString.h"

#include "../../throw.h"

#include <windows.h>
//
#include <stringapiset.h>

#include <cwchar>

namespace mdnscpp
{
  std::wstring toWideString(const std::string &input)
  {
    std::wstring result;
    if (input.size())
    {
      int count = MultiByteToWideChar(
          CP_UTF8, 0, input.data(), static_cast<int>(input.length()), NULL, 0);
      if (count <= 0)
        MDNSCPP_THROW(std::invalid_argument, "Invalid utf-8 input.");
      result.resize(count);
      if (MultiByteToWideChar(CP_UTF8, 0, input.data(),
              static_cast<int>(input.length()), result.data(), count) <= 0)
        MDNSCPP_THROW(std::invalid_argument, "Invalid utf-8 input.");
    }

    return result;
  }
} // namespace mdnscpp