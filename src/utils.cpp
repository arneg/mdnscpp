#include <mdnscpp/utils.h>

#include <algorithm>

namespace mdnscpp
{
  std::vector<BrowseResult> getSortedList(
      const std::unordered_set<std::shared_ptr<BrowseResult>> &list)
  {
    std::vector<BrowseResult> result;

    result.reserve(list.size());

    for (const auto &it : list)
    {
      result.emplace_back(*it);
    }

    std::sort(result.begin(), result.end());

    return result;
  }
} // namespace mdnscpp
