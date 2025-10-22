#include "debug.h"
#include <algorithm>
#include <mdnscpp/Browser.h>

#include "throw.h"

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

    ~ResultRemovalContext() { browser_->removeResult(result_); }
  };

  Browser::Browser(const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol, EventLoop &loop)
      : notifyResultsTimer_(loop.createTimeout(EventLoop::TimeoutDisabled{},
            [this](EventLoop::Timeout &) { notifyResultsChanged(); })),
        type_(type), protocol_(protocol), onResultsChanged_(onResultsChanged),
        domain_(domain), interfaceIndex_(interfaceIndex),
        ipProtocol_(ipProtocol)
  {
  }

  const std::string &Browser::getType() const { return type_; }

  const std::string &Browser::getProtocol() const { return protocol_; }

  const std::string &Browser::getDomain() const { return domain_; }

  size_t Browser::getInterface() const { return interfaceIndex_; }

  IPProtocol Browser::getIPProtocol() const { return ipProtocol_; }

  const std::unordered_set<std::shared_ptr<BrowseResult>>
  Browser::getResults() const
  {
    return results_;
  }

  std::shared_ptr<void> Browser::addResult(std::shared_ptr<BrowseResult> result)
  {
    insertResult(result);
    return std::make_shared<ResultRemovalContext>(
        getSharedFromThis(), std::move(result));
  }

  void Browser::insertResult(std::shared_ptr<BrowseResult> result)
  {

    MDNSCPP_DEBUG_ASSERT(results_.find(result) != results_.end());
    results_.insert(result);
    triggerNotifyResultsChanged();
  }

  void Browser::removeResult(std::shared_ptr<BrowseResult> result)
  {
    auto count = results_.erase(result);

    MDNSCPP_ASSERT(count == 1);
    triggerNotifyResultsChanged();
  }

  void Browser::notifyResultsChanged()
  {
    notifyResultsTimerPending_ = false;
    if (!onResultsChanged_)
      return;

#ifdef __cpp_exception
    try
    {
#endif
      onResultsChanged_(getSharedFromThis());
#ifdef __cpp_exception
    }
    catch (const std::exception &e)
    {
      MDNSCPP_ERROR << "onResultsChanged() threw an exception: " << e.what()
                    << MDNSCPP_ENDL;
    }
#endif
  }

  void Browser::triggerNotifyResultsChanged()
  {
    if (!onResultsChanged_)
      return;
    if (notifyResultsTimerPending_)
      return;
    notifyResultsTimerPending_ = true;
    notifyResultsTimer_->updateImmediate();
  }
} // namespace mdnscpp
