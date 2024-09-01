#pragma once

#include <string>
#include <utility>
#include <vector>

namespace mdnscpp
{

  using TxtRecord = std::pair<std::string, std::string>;

  class BrowseResult
  {
  public:
    const std::vector<TxtRecord> &getTxtRecords() const;
    const std::string &getType() const;
    const std::string &getName() const;
    const std::string &getDomain() const;
    size_t getInterface() const;

  private:
    std::vector<TxtRecord> txtRecords_;
    std::string type_;
    std::string name_;
    std::string domain_;
    size_t interface_;
  };
} // namespace mdnscpp
