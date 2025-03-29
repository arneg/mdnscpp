#include <mdnscpp/CallQueue.h>

#include <iostream>
#include <utility>

namespace mdnscpp
{
  CallQueue::CallQueue(EventLoop &loop)
      : async_(loop.createAsync([this]() { executeCallbacks(); }))
  {
  }

  std::vector<CallQueue::Callback> CallQueue::getCallbacksToRun()
  {
    std::lock_guard lock(mutex_);
    return std::move(callbacks_);
  }

  void CallQueue::executeCallbacks()
  {
    auto cbs = getCallbacksToRun();
    for (auto cb : cbs)
    {
      try
      {
        cb();
      }
      catch (std::exception &err)
      {
        // TODO: report error
      }
    }
  }

  void CallQueue::schedule(Callback callback)
  {
    {
      std::lock_guard lock(mutex_);
      callbacks_.emplace_back(std::move(callback));
    }
    async_->trigger();
  }

  std::shared_ptr<CallQueue> CallQueue::create(EventLoop &loop)
  {
    return std::make_shared<CallQueue>(loop);
  }
} // namespace mdnscpp