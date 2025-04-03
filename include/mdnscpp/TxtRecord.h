#pragma once

#include <optional>
#include <string>

namespace mdnscpp
{
  struct TxtRecord
  {
    std::string key;
    std::optional<std::string> value;

    bool operator==(const TxtRecord &other) const;
    bool operator!=(const TxtRecord &other) const;
  };
} // namespace mdnscpp