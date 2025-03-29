#pragma once

#include "BrowseResult.h"
#include <memory>
#include <unordered_set>
#include <vector>

namespace mdnscpp
{
  std::vector<BrowseResult> getSortedList(
      const std::unordered_set<std::shared_ptr<BrowseResult>> &list);
} // namespace mdnscpp