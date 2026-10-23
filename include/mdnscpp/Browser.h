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

    /**
     * The platform error that stopped this browser, or 0 while it runs. A
     * browser with an error has no results and does not browse again; create
     * a new one to retry.
     */
    int getError() const;

    /**
     * Holds a result in the browser. Destroying the last shared_ptr
     * removes it. The result can be read to build a replacement with
     * updated fields.
     */
    class ResultRemovalContext
    {
    public:
      ResultRemovalContext(std::shared_ptr<Browser> browser,
          std::shared_ptr<BrowseResult> result);
      ~ResultRemovalContext();

      ResultRemovalContext(const ResultRemovalContext &) = delete;
      ResultRemovalContext &operator=(const ResultRemovalContext &) = delete;

      std::shared_ptr<BrowseResult> getResult() const { return result_; }

    private:
      std::weak_ptr<Browser> browser_;
      std::shared_ptr<BrowseResult> result_;
    };

    std::shared_ptr<ResultRemovalContext> addResult(
        std::shared_ptr<BrowseResult> result);

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
    int error_ = 0;

    virtual std::shared_ptr<Browser> getSharedFromThis() = 0;
    void notifyResultsChanged();
    void triggerNotifyResultsChanged();
  };
} // namespace mdnscpp
