#pragma once

#include <optional>
#include <string>

namespace mdnscpp
{
  struct TxtRecord
  {
    std::string key;
    std::optional<std::string> value;
  };
} // namespace mdnscpp