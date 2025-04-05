#include <mdnscpp/PollLoop.h>

#include <limits>
#include <poll.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>

static struct timeval getNow()
{
  struct timeval result;

  // Can not fail?
  gettimeofday(&result, nullptr);

  return result;
}

static struct timeval addTime(const struct timeval &a, const struct timeval &b)
{
  struct timeval result;

  timeradd(&a, &b, &result);

  return result;
}

static struct timeval subTime(const struct timeval &a, const struct timeval &b)
{
  struct timeval result;

  timersub(&a, &b, &result);

  return result;
}

static size_t timevalToMs(const struct timeval &a)
{
  size_t result = a.tv_sec * 1000;

  return result + (a.tv_usec / 1000);
}

static bool isBefore(const struct timeval &lhs, const struct timeval &rhs)
{
  int result = timercmp(&lhs, &rhs, <);
  return !!result;
}

static bool isAfter(const struct timeval &lhs, const struct timeval &rhs)
{
  int result = timercmp(&lhs, &rhs, >);
  return !!result;
}

static bool isSameTime(const struct timeval &lhs, const struct timeval &rhs)
{
  int result = timercmp(&lhs, &rhs, ==);
  return !!result;
}

namespace mdnscpp
{
  bool PollLoop::runOnce()
  {
    size_t count = runTimeoutsUntil(getNow());

    updatePollFds();

    //std::cerr << "Calling poll with " << pollfds_.size() << " fds" << std::endl;

    int nready = poll(pollfds_.data(), pollfds_.size(), getSmallestTimeout());

    if (nready > 0)
    {
      for (size_t i = 0; i < pollfds_.size(); i++)
      {
        const auto &p = pollfds_[i];

        if (!p.revents)
          continue;

        auto *watch = polledWatches_[i];

        watch->updateReturnedEvents(fromPollEvent(p.revents));
      }

      for (size_t i = 0; i < pollfds_.size(); i++)
      {
        const auto &p = pollfds_[i];

        if (!p.revents)
          continue;

        auto *watch = polledWatches_[i];

        if (pollfdsInvalid_)
        {
          // The watch was removed.
          if (watches_.find(watch) == watches_.end())
            continue;
          // The watch was removed and then a new one created
          // with the same pointer
          if (toPollEvent(watch->getReturnedEvents()) != p.revents)
            continue;
        }

        watch->getCallback()(watch->getReturnedEvents());
      }

      if (processAsync_)
      {
        processAsync_ = false;

        for (auto &async : pollAsyncs_)
        {
          async->process();
        }

        pollAsyncs_.erase(
            std::remove_if(pollAsyncs_.begin(), pollAsyncs_.end(),
                [](const auto &async) { return async->shouldDeallocate(); }),
            pollAsyncs_.end());
      }
    }

    return count || pollfds_.size();
  }

  void PollLoop::run()
  {
    while (runOnce())
      ;
  }

  std::shared_ptr<EventLoop::Watch> PollLoop::createWatch(
      int fd, EventLoop::EventType events, Watch::Callback callback)
  {
    auto watch = std::make_shared<PollWatch>(*this, fd, callback);

    watch->update(events);

    return watch;
  }

  std::shared_ptr<EventLoop::Timeout> PollLoop::createTimeout(
      TimeoutState state, Timeout::Callback callback)
  {
    //std::cerr << "creating timeout" << std::endl;
    return std::make_shared<PollTimeout>(*this, state, callback);
  }

  std::shared_ptr<EventLoop::Async> PollLoop::createAsync(
      EventLoop::Async::Callback callback)
  {
    makeWakeupPipes();
    auto internalAsync = std::make_unique<PollAsync>(*this, callback);
    auto result = std::make_shared<Async>(*internalAsync);
    pollAsyncs_.emplace_back(std::move(internalAsync));
    return result;
  }

  constexpr bool PollLoop::CompareTimeval::operator()(
      const struct timeval &lhs, const struct timeval &rhs) const
  {
    int result = timercmp(&lhs, &rhs, <);
    return !!result;
  }

  PollLoop::PollWatch::PollWatch(
      PollLoop &loop, int fd, EventLoop::Watch::Callback callback)
      : EventLoop::Watch(fd, callback), loop_(loop)
  {
    loop_.addWatch(this);
  }

  PollLoop::PollWatch::~PollWatch() { loop_.removeWatch(this); }

  void PollLoop::PollWatch::update(EventLoop::EventType events)
  {
    updateRequestedEvents(events);
    loop_.updateWatch(this);
  }

  PollLoop::PollTimeout::PollTimeout(
      PollLoop &loop, TimeoutState state, Callback callback)
      : Timeout(state, callback), loop_(loop), time_{0, 0}
  {
    install();
  }

  PollLoop::PollTimeout::~PollTimeout() { uninstall(); }

  void PollLoop::PollTimeout::PollTimeout::update(TimeoutState state)
  {
    uninstall();
    state_ = state;
    install();
  }

  void PollLoop::PollTimeout::install()
  {
    if (std::holds_alternative<EventLoop::TimeoutDisabled>(state_))
      return;

    if (std::holds_alternative<EventLoop::TimeoutRelative>(state_))
    {
      time_ =
          addTime(getNow(), std::get<EventLoop::TimeoutRelative>(state_).time);
    }
    else
    {
      time_ = std::get<EventLoop::TimeoutAbsolute>(state_).time;
    }

    loop_.addTimeout(time_, this);
  }

  void PollLoop::PollTimeout::uninstall()
  {
    if (std::holds_alternative<EventLoop::TimeoutDisabled>(state_))
      return;

    loop_.removeTimeout(time_, this);
  }

  void PollLoop::PollTimeout::notify() { Timeout::notify(); }

  size_t PollLoop::runTimeoutsUntil(const struct timeval &time)
  {
    size_t count = 0;
    // Run all timers that have to run.
    for (auto it = timeouts_.begin(); it != timeouts_.end();
        it = timeouts_.begin())
    {
      if (isAfter(it->first, time))
        break;

      auto *timer = it->second;

      timeouts_.erase(it);
      timer->notify();
      count++;
    }

    return count;
  }

  int16_t PollLoop::toPollEvent(EventLoop::EventType events)
  {
    int16_t result = 0;

    if (events.hasRead())
      result |= POLLIN;
    if (events.hasWrite())
      result |= POLLOUT;
    if (events.hasDisconnect())
      result |= POLLHUP;
    if (events.hasError())
      result |= POLLERR;

    return result;
  }

  EventLoop::EventType PollLoop::fromPollEvent(int16_t pollevents)
  {
    EventLoop::EventType events;

    if (pollevents & POLLIN)
      events.setRead();
    if (pollevents & POLLOUT)
      events.setWrite();
    if (pollevents & POLLHUP)
      events.setDisconnect();
    if (pollevents & POLLERR)
      events.setError();

    return events;
  }

  void PollLoop::updatePollFds()
  {
    if (!pollfdsInvalid_)
      return;
    size_t size = watches_.size();
    pollfds_.resize(0);
    pollfds_.reserve(size);
    polledWatches_.resize(0);
    polledWatches_.reserve(size);

    for (PollWatch *watch : watches_)
    {
      auto requestedEvents = toPollEvent(watch->getRequestedEvents());

      if (!(requestedEvents & (POLLIN | POLLOUT)))
      {
        watch->updateReturnedEvents(fromPollEvent(0));
        continue;
      }

      pollfds_.push_back({watch->getFileDescriptor(), requestedEvents, 0});
      polledWatches_.push_back(watch);
    }

    //std::cerr << "Generated pollfds " << pollfds_.size() << std::endl;
    pollfdsInvalid_ = false;
  }

  int PollLoop::getSmallestTimeout()
  {
    auto nextTimeout = timeouts_.begin();

    if (nextTimeout == timeouts_.end())
      return -1;

    const auto now = getNow();

    if (isBefore(nextTimeout->first, now))
      return 0;

    auto ms = timevalToMs(subTime(nextTimeout->first, now));

    return std::numeric_limits<int>::max() < ms
               ? std::numeric_limits<int>::max()
               : static_cast<int>(ms);
  }

  void PollLoop::addTimeout(const struct timeval &t, PollTimeout *timeout)
  {
    timeouts_.insert({t, timeout});
  }

  void PollLoop::removeTimeout(const struct timeval &t, PollTimeout *timeout)
  {
    for (auto it = timeouts_.find(t);
        it != timeouts_.end() && isSameTime(t, it->first); it++)
    {
      if (it->second == timeout)
      {
        timeouts_.erase(it);
        return;
      }
    }
  }

  void PollLoop::addWatch(PollWatch *watch)
  {
    pollfdsInvalid_ = true;
    watches_.insert(watch);
  }

  void PollLoop::removeWatch(PollWatch *watch)
  {
    pollfdsInvalid_ = true;
    watches_.erase(watch);
  }

  void PollLoop::updateWatch(PollWatch *watch) { pollfdsInvalid_ = true; }

  PollLoop::~PollLoop() { removeWakeupPipes(); }

  void PollLoop::makeWakeupPipes()
  {
    if (wakeupPipes_[0])
      return;
    int ok = pipe(wakeupPipes_);
#ifdef __cpp_exceptions
    if (ok)
      throw std::runtime_error("Failed to create wakeup pipes.");
#else
    assert(!ok);
#endif
    wakeupWatch_ =
        createWatch(wakeupPipes_[0], EventType::TYPE_READ, [&](auto type) {
          char buf[128];
          read(wakeupPipes_[0], buf, 128);
          // process async
          processAsync_ = true;
        });
  }

  void PollLoop::removeWakeupPipes()
  {
    wakeupWatch_ = nullptr;
    if (wakeupPipes_[0])
    {
      close(wakeupPipes_[0]);
      close(wakeupPipes_[1]);
    }
  }

  PollLoop::PollAsync::PollAsync(PollLoop &loop, Callback callback)
      : loop_(loop), InternalAsync(callback)
  {
  }

  bool PollLoop::PollAsync::trigger(bool deallocate)
  {
    if (InternalAsync::trigger(deallocate))
    {
      write(loop_.wakeupPipes_[1], "\0", 1);
      return true;
    }
    else
    {
      return false;
    }
  }
} // namespace mdnscpp
