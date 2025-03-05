#include <algorithm>
#include <iostream>
#include <mdnscpp/Browser.h>
#include <stdexcept>

namespace mdnscpp
{

  class ResultRemovalContext
  {
    std::shared_ptr<Browser> browser_;
    std::shared_ptr<BrowseResult> result_;

  public:
    ResultRemovalContext(
        std::shared_ptr<Browser> browser, std::shared_ptr<BrowseResult> result)
        : browser_(browser), result_(result)
    {
    }

    ~ResultRemovalContext()
    {
      // remove result
      browser_->removeResult(result_);
    }
  };

  Browser::Browser(const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interface, IPProtocol ipProtocol)
      : type_(type), protocol_(protocol), onResultsChanged_(onResultsChanged),
        domain_(domain), interface_(interface), ipProtocol_(ipProtocol)
  {
  }

  const std::string &Browser::getType() const { return type_; }

  const std::string &Browser::getProtocol() const { return protocol_; }

  const std::string &Browser::getDomain() const { return domain_; }

  size_t Browser::getInterface() const { return interface_; }

  IPProtocol Browser::getIPProtocol() const { return ipProtocol_; }

  const std::unordered_set<std::shared_ptr<BrowseResult>>
  Browser::getResults() const
  {
    return results_;
  }

  std::shared_ptr<void> Browser::addResult(std::shared_ptr<BrowseResult> result)
  {
    if (results_.find(result) != results_.end())
    {
      throw std::logic_error("Result already found. Unexpected.");
    }

    results_.insert(result);
    notifyResultsChanged();
    return std::make_shared<ResultRemovalContext>(
        getSharedFromThis(), std::move(result));
  }

  void Browser::removeResult(std::shared_ptr<BrowseResult> result)
  {
    if (results_.erase(result) != 1)
    {
      throw std::logic_error("Entry not found.");
    }
    notifyResultsChanged();
  }

  void Browser::notifyResultsChanged()
  {
    if (!onResultsChanged_)
      return;

    try
    {
      onResultsChanged_(getSharedFromThis());
    }
    catch (const std::exception &e)
    {
      std::cerr << "onResultsChanged() threw an exception: " << e.what()
                << std::endl;
    }
  }
} // namespace mdnscpp
