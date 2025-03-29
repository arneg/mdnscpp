#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_set>

#include <poll.h>

#include "EventLoop.h"

namespace mdnscpp
{
  class PollLoop : public EventLoop
  {
  public:
    bool runOnce();
    void run();
    ~PollLoop();
    std::shared_ptr<Watch> createWatch(
        int fd, EventType events, Watch::Callback callback) override;
    std::shared_ptr<Timeout> createTimeout(
        TimeoutState state, Timeout::Callback callback) override;
    std::shared_ptr<Async> createAsync(Async::Callback) override;

  private:
    struct CompareTimeval
    {
      constexpr bool operator()(
          const struct timeval &lhs, const struct timeval &rhs) const;
    };

    class PollWatch : public Watch
    {
    public:
      PollWatch(PollLoop &loop, int fd, Callback callback);
      ~PollWatch();
      void update(EventType events) override;

      using Watch::updateReturnedEvents;

    private:
      PollLoop &loop_;
    };

    class PollTimeout : public Timeout
    {
    public:
      PollTimeout(PollLoop &loop, TimeoutState state, Callback callback);
      ~PollTimeout();
      void update(TimeoutState state) override;
      void notify();

    private:
      PollLoop &loop_;
      struct timeval time_;

      void install();
      void uninstall();
    };

    class PollAsync : public InternalAsync
    {
    public:
      PollAsync(PollLoop &loop, Callback callback);
      bool trigger(bool deallocate) override;

    private:
      PollLoop &loop_;
    };

    static int16_t toPollEvent(EventType events);
    static EventType fromPollEvent(int16_t pollevents);

    size_t runTimeoutsUntil(const struct timeval &time);
    void updatePollFds();
    int getSmallestTimeout();
    void makeWakeupPipes();
    void removeWakeupPipes();

    void addTimeout(const struct timeval &t, PollTimeout *timeout);
    void removeTimeout(const struct timeval &t, PollTimeout *timeout);

    void addWatch(PollWatch *watch);
    void removeWatch(PollWatch *watch);
    void updateWatch(PollWatch *watch);

    std::multimap<struct timeval, PollTimeout *, CompareTimeval> timeouts_;
    std::unordered_set<PollWatch *> watches_;
    std::vector<struct pollfd> pollfds_;
    std::vector<PollWatch *> polledWatches_;
    std::vector<std::unique_ptr<PollAsync>> pollAsyncs_;
    std::shared_ptr<Watch> wakeupWatch_ = nullptr;
    int wakeupPipes_[2] = {0, 0};
    bool processAsync_ = false;
    bool pollfdsInvalid_ = true;
  };
} // namespace mdnscpp
