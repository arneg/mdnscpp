#pragma once

#include <memory>

#include <uv.h>

#include "EventLoop.h"

namespace mdnscpp
{
  class LibuvLoop : public EventLoop
  {
  public:
    void run();
    LibuvLoop(uv_loop_t *loop = uv_default_loop());
    ~LibuvLoop();

    uv_loop_t *getUvLoop() const;

    std::shared_ptr<Watch> createWatch(
        int fd, EventType events, Watch::Callback callback) override;
    std::shared_ptr<Timeout> createTimeout(
        TimeoutState state, Timeout::Callback callback) override;
    std::shared_ptr<Async> createAsync(Async::Callback) override;

    uint64_t now() const override;

  private:
    class LibuvWatch;
    class LibuvTimeout;

    class LibuvPoll
    {
    public:
      LibuvPoll(LibuvWatch *watch, int fd);

      void update(int events);
      void close();
      void onEvents(int status, int events);

    private:
      LibuvWatch *watch_;
      uv_poll_t uv_poll_;
      int fd_;
    };

    class LibuvTimer
    {
    public:
      LibuvTimer(LibuvTimeout *timeout);
      void start(uint64_t timeout);
      void stop();
      void close();
      void timerCallback();

    private:
      LibuvTimeout *timeout_;
      uv_timer_t uv_timer_;
    };

    class LibuvWatch : public Watch
    {
    public:
      LibuvWatch(LibuvLoop &loop, int fd, Callback callback);
      ~LibuvWatch();
      void update(EventType events) override;

      uv_loop_t *getUvLoop() const;

      using Watch::updateReturnedEvents;

    private:
      LibuvLoop &loop_;
      LibuvPoll *poll_;
    };

    class LibuvTimeout : public Timeout
    {
    public:
      LibuvTimeout(LibuvLoop &loop, TimeoutState state, Callback callback);
      ~LibuvTimeout();
      void update(TimeoutState state) override;
      void notify();

      uv_loop_t *getUvLoop() const;

    private:
      LibuvLoop &loop_;
      struct timeval time_;
      LibuvTimer *timer_;

      void install();
      void uninstall();
    };

    class LibuvAsync : public InternalAsync
    {
    public:
      LibuvAsync(LibuvLoop &loop, Callback callback);
      bool trigger(bool deallocate) override;

    private:
      static void asyncCallback(uv_async_t *);
      static void closeCallback(uv_handle_t *);

      LibuvLoop &loop_;
      uv_async_t uv_async_;
    };

    uv_loop_t *uv_loop_;
  };
} // namespace mdnscpp
