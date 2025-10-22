#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

#include "BrowseResult.h"
#include "EventLoop.h"
#include "Types.h"

namespace mdnscpp
{

  class Browser
  {
  public:
    using ResultsChangedCallback =
        std::function<void(std::shared_ptr<Browser>)>;

    Browser(const std::string &type, const std::string &protocol,
        ResultsChangedCallback onResultsChanged, const std::string &domain,
        size_t interfaceIndex, IPProtocol ipProtocol, EventLoop &loop);
    virtual ~Browser() = default;

    const std::string &getType() const;
    const std::string &getProtocol() const;
    const std::string &getDomain() const;
    size_t getInterface() const;
    IPProtocol getIPProtocol() const;
    const std::unordered_set<std::shared_ptr<BrowseResult>> getResults() const;

    std::shared_ptr<void> addResult(std::shared_ptr<BrowseResult> result);

    void insertResult(std::shared_ptr<BrowseResult> result);
    void removeResult(std::shared_ptr<BrowseResult> result);

  protected:
    std::shared_ptr<EventLoop::Timeout> notifyResultsTimer_;
    std::unordered_set<std::shared_ptr<BrowseResult>> results_;
    const std::string type_;
    const std::string protocol_;
    ResultsChangedCallback onResultsChanged_;
    const std::string domain_;
    const size_t interfaceIndex_;
    const IPProtocol ipProtocol_;
    bool notifyResultsTimerPending_ = false;

    virtual std::shared_ptr<Browser> getSharedFromThis() = 0;
    void notifyResultsChanged();
    void triggerNotifyResultsChanged();
  };
} // namespace mdnscpp
