#pragma once

#include "EventLoop.h"
#include <memory>

namespace mdnscpp
{
  class CallQueue
  {
  public:
    using Callback = std::function<void(void)>;

    virtual ~CallQueue() = default;
    CallQueue(EventLoop &loop);

    void schedule(Callback callback);

    static std::shared_ptr<CallQueue> create(EventLoop &loop);

  private:
    std::mutex mutex_;
    std::vector<Callback> callbacks_;
    std::shared_ptr<EventLoop::Async> async_;

    std::vector<Callback> getCallbacksToRun();
    void executeCallbacks();
  };
} // namespace mdnscpp
