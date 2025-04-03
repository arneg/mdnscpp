#include <mdnscpp/LibuvLoop.h>

#include <limits>

#include <iostream>

namespace mdnscpp
{
  void LibuvLoop::run() { uv_run(uv_default_loop(), UV_RUN_DEFAULT); }

  LibuvLoop::LibuvLoop(uv_loop_t *loop) : uv_loop_(loop) {}

  uv_loop_t *LibuvLoop::getUvLoop() const { return uv_loop_; }

  std::shared_ptr<EventLoop::Watch> LibuvLoop::createWatch(
      int fd, EventLoop::EventType events, Watch::Callback callback)
  {
    return nullptr;
  }

  std::shared_ptr<EventLoop::Timeout> LibuvLoop::createTimeout(
      TimeoutState state, Timeout::Callback callback)
  {
    return nullptr;
  }

  uint64_t LibuvLoop::now() const { return uv_now(uv_loop_); }

  std::shared_ptr<EventLoop::Async> LibuvLoop::createAsync(
      EventLoop::Async::Callback callback)
  {
    auto *internalAsync = new LibuvAsync(*this, std::move(callback));
    return std::make_shared<Async>(*internalAsync);
  }

  LibuvLoop::LibuvWatch::LibuvWatch(
      LibuvLoop &loop, int fd, EventLoop::Watch::Callback callback)
      : EventLoop::Watch(fd, callback), loop_(loop)
  {
  }

  LibuvLoop::LibuvWatch::~LibuvWatch() {}

  void LibuvLoop::LibuvWatch::update(EventLoop::EventType events)
  {
    updateRequestedEvents(events);
  }

  LibuvLoop::LibuvTimeout::LibuvTimeout(
      LibuvLoop &loop, TimeoutState state, Callback callback)
      : Timeout(state, callback), loop_(loop), time_{0, 0}
  {
    install();
  }

  LibuvLoop::LibuvTimeout::~LibuvTimeout() { uninstall(); }

  void LibuvLoop::LibuvTimeout::LibuvTimeout::update(TimeoutState state)
  {
    uninstall();
    state_ = state;
    install();
  }

  void LibuvLoop::LibuvTimeout::install()
  {
    if (std::holds_alternative<EventLoop::TimeoutDisabled>(state_))
      return;
  }

  void LibuvLoop::LibuvTimeout::uninstall()
  {
    if (std::holds_alternative<EventLoop::TimeoutDisabled>(state_))
      return;
  }

  void LibuvLoop::LibuvTimeout::notify() { Timeout::notify(); }

  LibuvLoop::~LibuvLoop() {}

  LibuvLoop::LibuvAsync::LibuvAsync(LibuvLoop &loop, Callback callback)
      : loop_(loop), InternalAsync(callback)
  {
    if (uv_async_init(loop.getUvLoop(), &uv_async_, &asyncCallback))
    {
      throw std::runtime_error("uv_async_init failed.");
    }
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(&uv_async_), this);
  }

  void LibuvLoop::LibuvAsync::asyncCallback(uv_async_t *handle)
  {
    LibuvLoop::LibuvAsync *async = reinterpret_cast<LibuvLoop::LibuvAsync *>(
        uv_handle_get_data(reinterpret_cast<uv_handle_t *>(handle)));
    if (async->shouldDeallocate())
    {
      uv_close(reinterpret_cast<uv_handle_t *>(handle), closeCallback);
    }
    else
    {
      async->process();
    }
  }

  void LibuvLoop::LibuvAsync::closeCallback(uv_handle_t *handle)
  {
    LibuvLoop::LibuvAsync *async =
        reinterpret_cast<LibuvLoop::LibuvAsync *>(uv_handle_get_data(handle));

    delete (async);
  }

  bool LibuvLoop::LibuvAsync::trigger(bool deallocate)
  {
    if (InternalAsync::trigger(deallocate))
    {
      uv_async_send(&uv_async_);
      return true;
    }

    return false;
  }
} // namespace mdnscpp
