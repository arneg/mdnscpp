#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

#include <sys/time.h>

namespace mdnscpp
{
  class EventLoop
  {
  public:
    EventLoop() = default;

    class EventType
    {
    public:
      EventType() = default;

      bool hasRead() const;
      bool hasWrite() const;
      bool hasDisconnect() const;
      bool hasError() const;

      EventType operator|(const EventType &other) const;
      EventType operator&(const EventType &other) const;
      bool operator!() const;

      void setRead();
      void setWrite();
      void setDisconnect();
      void setError();

      static const EventType READ;
      static const EventType WRITE;
      static const EventType DISCONNECT;
      static const EventType ERROR;

    private:
      explicit EventType(uint8_t flags);

      uint8_t flags_ = 0;
    };

    class Watch
    {
    public:
      using Callback = std::function<void(EventType)>;

      Watch(int fd, Callback callback);
      virtual ~Watch() = default;

      virtual void update(EventType events) = 0;

      int getFileDescriptor() const;
      EventType getReturnedEvents() const;
      EventType getRequestedEvents() const;
      Callback getCallback() const;

    protected:
      const int fd_;
      const Callback callback_;
      EventType requestedEvents_;
      EventType returnedEvents_;

      void updateReturnedEvents(EventType events);
      void updateRequestedEvents(EventType events);
    };

    struct TimeoutDisabled
    {
    };
    struct TimeoutAbsolute
    {
      struct timeval time;
    };
    struct TimeoutRelative
    {
      struct timeval time;
    };

    using TimeoutState =
        std::variant<TimeoutDisabled, TimeoutAbsolute, TimeoutRelative>;

    class Timeout
    {
    public:
      using Callback = std::function<void(Timeout &)>;

      Timeout(TimeoutState state, Callback callback);
      virtual ~Timeout() = default;
      virtual void update(TimeoutState state) = 0;

    protected:
      const Callback callback_;

      void notify();

      TimeoutState state_{TimeoutDisabled{}};
    };

    class CallQueue
    {
    public:
      using Callback = std::function<void(void)>;

      CallQueue();
      virtual ~CallQueue() = default;

      void schedule(Callback callback);

    private:
      std::mutex mutex_;
      std::vector<Callback> callbacks_;

      void executeCallbacks();
    };

    virtual std::shared_ptr<Watch> createWatch(
        int fd, EventType events, Watch::Callback callback) = 0;
    virtual std::shared_ptr<Timeout> createTimeout(
        TimeoutState state, Timeout::Callback callback) = 0;
    virtual std::shared_ptr<CallQueue> createCallQueue() = 0;

  private:
  };
} // namespace mdnscpp
