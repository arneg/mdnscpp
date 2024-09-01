#include <mdnscpp/BrowseResult.h>

namespace mdnscpp
{
  const std::vector<TxtRecord> &BrowseResult::getTxtRecords() const
  {
    return txtRecords_;
  }

  const std::string &BrowseResult::getType() const { return type_; }

  const std::string &BrowseResult::getName() const { return name_; }

  const std::string &BrowseResult::getDomain() const { return domain_; }

  size_t BrowseResult::getInterface() const { return interface_; }
}; // namespace mdnscpp
