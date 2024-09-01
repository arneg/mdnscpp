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
    std::shared_ptr<Watch> createWatch(
        int fd, EventType events, Watch::Callback callback) override;
    std::shared_ptr<Timeout> createTimeout(
        TimeoutState state, Timeout::Callback callback) override;
    std::shared_ptr<CallQueue> createCallQueue() override;

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

    static int16_t toPollEvent(EventType events);
    static EventType fromPollEvent(int16_t pollevents);

    size_t runTimeoutsUntil(const struct timeval &time);
    void updatePollFds();
    int getSmallestTimeout();

    void addTimeout(const struct timeval &t, PollTimeout *timeout);
    void removeTimeout(const struct timeval &t, PollTimeout *timeout);

    void addWatch(PollWatch *watch);
    void removeWatch(PollWatch *watch);
    void updateWatch(PollWatch *watch);

    std::multimap<struct timeval, PollTimeout *, CompareTimeval> timeouts_;
    std::unordered_set<PollWatch *> watches_;
    std::vector<struct pollfd> pollfds_;
    std::vector<PollWatch *> polledWatches_;
    bool pollfdsInvalid_ = true;
  };
} // namespace mdnscpp
