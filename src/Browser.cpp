#include "debug.h"
#include "guardCallback.h"
#include "throw.h"
#include <algorithm>
#include <mdnscpp/Browser.h>
#include <mdnscpp/validation.h>

namespace mdnscpp
{
  Browser::ResultRemovalContext::ResultRemovalContext(
      std::shared_ptr<Browser> browser, std::shared_ptr<BrowseResult> result)
      : browser_(std::move(browser)), result_(std::move(result))
  {
  }

  Browser::ResultRemovalContext::~ResultRemovalContext()
  {
    if (auto browser = browser_.lock())
      browser->removeResult(result_);
  }

  Browser::Browser(const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol, EventLoop &loop)
      : notifyResultsTimer_(loop.createTimeout(EventLoop::TimeoutDisabled{},
            [this](EventLoop::Timeout &) { notifyResultsChanged(); })),
        type_(type), protocol_(protocol), onResultsChanged_(onResultsChanged),
        domain_(domain), interfaceIndex_(interfaceIndex),
        ipProtocol_(ipProtocol)
  {
    if (auto error =
            validateBrowseArguments(type, protocol, domain, interfaceIndex))
    {
      if (error->argument == BrowseArgumentError::Argument::InterfaceIndex)
      {
        MDNSCPP_THROW(std::out_of_range, error->message);
      }
      else
      {
        MDNSCPP_THROW(std::invalid_argument, error->message);
      }
    }
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

  int Browser::getError() const { return error_; }

  std::shared_ptr<Browser::ResultRemovalContext> Browser::addResult(
      std::shared_ptr<BrowseResult> result)
  {
    insertResult(result);
    return std::make_shared<ResultRemovalContext>(
        getSharedFromThis(), std::move(result));
  }

  void Browser::insertResult(std::shared_ptr<BrowseResult> result)
  {
    // A result must not be inserted twice.
    MDNSCPP_DEBUG_ASSERT(results_.find(result) == results_.end());
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

    guardCallback("Browser::notifyResultsChanged",
        [&] { onResultsChanged_(getSharedFromThis()); });
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
