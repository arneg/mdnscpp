#include <mdnscpp/LibuvLoop.h>

#include <cassert>
#include <iostream>
#include <limits>

namespace mdnscpp
{
  /**
   * Helpers
   */

  EventLoop::EventType fromUvEvents(int events)
  {
    EventLoop::EventType result;

    if (events & UV_READABLE)
      result.setRead();
    if (events & UV_WRITABLE)
      result.setWrite();
    if (events & UV_DISCONNECT)
      result.setDisconnect();

    return result;
  }

  int toUvEvents(EventLoop::EventType events)
  {
    int result = 0;

    if (events.hasRead())
      result |= UV_READABLE;
    if (events.hasWrite())
      result |= UV_WRITABLE;
    if (events.hasDisconnect())
      result |= UV_DISCONNECT;

    return result;
  }

  /**
   * LibuvLoop
   */

  void LibuvLoop::run() { uv_run(uv_default_loop(), UV_RUN_DEFAULT); }

  LibuvLoop::LibuvLoop(uv_loop_t *loop) : uv_loop_(loop) {}

  uv_loop_t *LibuvLoop::getUvLoop() const { return uv_loop_; }

  std::shared_ptr<EventLoop::Watch> LibuvLoop::createWatch(
      int fd, EventLoop::EventType events, Watch::Callback callback)
  {
    auto watch = std::make_shared<LibuvWatch>(*this, fd, callback);
    watch->update(events);
    return watch;
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

  LibuvLoop::~LibuvLoop() {}

  /**
   * LibuvPoll
   */

  LibuvLoop::LibuvPoll::LibuvPoll(LibuvWatch *watch, int fd)
      : watch_(watch), fd_(fd)
  {
    uv_poll_init(watch->getUvLoop(), &uv_poll_, fd);
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(&uv_poll_), this);
  }

  void LibuvLoop::LibuvPoll::update(int events)
  {
    if (events)
    {
      uv_poll_start(
          &uv_poll_, events, [](uv_poll_t *handle, int status, int events) {
            reinterpret_cast<LibuvPoll *>(handle->data)
                ->onEvents(status, events);
          });
    }
    else
    {
      uv_poll_stop(&uv_poll_);
    }
  }

  void LibuvLoop::LibuvPoll::close()
  {
    assert(watch_);
    watch_ = nullptr;
    uv_poll_stop(&uv_poll_);
    uv_close(
        reinterpret_cast<uv_handle_t *>(&uv_poll_), [](uv_handle_t *handle) {
          delete reinterpret_cast<LibuvPoll *>(handle->data);
        });
  }

  void LibuvLoop::LibuvPoll::onEvents(int status, int events)
  {
    if (!watch_)
      return;

    auto callback = watch_->getCallback();

    EventLoop::EventType e =
        (status < 0) ? EventLoop::EventType::TYPE_ERROR : fromUvEvents(events);

    callback(e);
  }

  /**
   * LibuvWatch
   */

  LibuvLoop::LibuvWatch::LibuvWatch(
      LibuvLoop &loop, int fd, EventLoop::Watch::Callback callback)
      : EventLoop::Watch(fd, callback), loop_(loop), poll_(nullptr)
  {
    poll_ = new LibuvPoll(this, fd);
  }

  LibuvLoop::LibuvWatch::~LibuvWatch()
  {
    if (poll_)
    {
      poll_->close();
      poll_ = nullptr;
    }
  }

  void LibuvLoop::LibuvWatch::update(EventLoop::EventType events)
  {
    updateRequestedEvents(events);
    poll_->update(toUvEvents(events));
  }

  uv_loop_t *LibuvLoop::LibuvWatch::getUvLoop() const
  {
    return loop_.getUvLoop();
  }

  /**
   * LibuvTimeout
   */

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

  /**
   * LibuvAsync
   */

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
