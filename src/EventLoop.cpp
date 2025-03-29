#include <mdnscpp/EventLoop.h>

static const uint8_t EVENT_TYPE_READ = 1;
static const uint8_t EVENT_TYPE_WRITE = 2;
static const uint8_t EVENT_TYPE_DISCONNECT = 4;
static const uint8_t EVENT_TYPE_ERROR = 8;

namespace mdnscpp
{
  bool EventLoop::EventType::hasRead() const
  {
    return !!(flags_ & EVENT_TYPE_READ);
  }

  bool EventLoop::EventType::hasWrite() const
  {
    return !!(flags_ & EVENT_TYPE_WRITE);
  }

  bool EventLoop::EventType::hasDisconnect() const
  {
    return !!(flags_ & EVENT_TYPE_DISCONNECT);
  }

  bool EventLoop::EventType::hasError() const
  {
    return !!(flags_ & EVENT_TYPE_ERROR);
  }

  EventLoop::EventType EventLoop::EventType::operator|(
      const EventLoop::EventType &other) const
  {
    const uint8_t flags = flags_ | other.flags_;
    return EventLoop::EventType{flags};
  }

  EventLoop::EventType EventLoop::EventType::operator&(
      const EventLoop::EventType &other) const
  {
    const uint8_t flags = flags_ & other.flags_;
    return EventLoop::EventType{flags};
  }

  bool EventLoop::EventType::operator!() const { return flags_ == 0; }

  void EventLoop::EventType::setRead() { flags_ |= EVENT_TYPE_READ; }

  void EventLoop::EventType::setWrite() { flags_ |= EVENT_TYPE_WRITE; }

  void EventLoop::EventType::setDisconnect()
  {
    flags_ |= EVENT_TYPE_DISCONNECT;
  }

  void EventLoop::EventType::setError() { flags_ |= EVENT_TYPE_ERROR; }

  EventLoop::EventType::EventType(uint8_t flags) : flags_(flags) {}

  const EventLoop::EventType EventLoop::EventType::TYPE_READ{EVENT_TYPE_READ};
  const EventLoop::EventType EventLoop::EventType::TYPE_WRITE{EVENT_TYPE_WRITE};
  const EventLoop::EventType EventLoop::EventType::TYPE_DISCONNECT{
      EVENT_TYPE_DISCONNECT};
  const EventLoop::EventType EventLoop::EventType::TYPE_ERROR{EVENT_TYPE_ERROR};

  EventLoop::Watch::Watch(int fd, EventLoop::Watch::Callback callback)
      : fd_(fd), callback_(callback)
  {
  }

  int EventLoop::Watch::getFileDescriptor() const { return fd_; }

  EventLoop::EventType EventLoop::Watch::getReturnedEvents() const
  {
    return returnedEvents_;
  }

  EventLoop::EventType EventLoop::Watch::getRequestedEvents() const
  {
    return requestedEvents_;
  }

  EventLoop::Watch::Callback EventLoop::Watch::getCallback() const
  {
    return callback_;
  }

  void EventLoop::Watch::updateReturnedEvents(EventType events)
  {
    returnedEvents_ = events;
  }

  void EventLoop::Watch::updateRequestedEvents(EventType events)
  {
    requestedEvents_ = events;
  }

  EventLoop::Timeout::Timeout(
      TimeoutState state, EventLoop::Timeout::Callback callback)
      : state_(state), callback_(callback)
  {
  }

  void EventLoop::Timeout::notify()
  {
    state_ = TimeoutDisabled{};
    callback_(*this);
  }

  EventLoop::InternalAsync::InternalAsync(
      EventLoop::InternalAsync::Callback callback)
      : callback_(callback)
  {
  }

  EventLoop::Async::Async(EventLoop::InternalAsync &internalAsync)
      : internalAsync_(internalAsync)
  {
  }

  bool EventLoop::InternalAsync::shouldDeallocate()
  {
    return state_.load() == State::DEALLOCATE;
  }

  bool EventLoop::InternalAsync::trigger(bool deallocate)
  {
    if (deallocate)
    {
      state_ = State::DEALLOCATE;
      return true;
    }
    else
    {
      auto currentState = state_.load();

      if (currentState == State::INACTIVE)
      {
        return state_.compare_exchange_strong(currentState, State::ACTIVE);
      }
      else
      {
        return false;
      }
    }
  }

  void EventLoop::InternalAsync::process()
  {
    auto currentState = state_.load();
    if (currentState == State::ACTIVE)
    {
      // FIXME: could be weaker
      if (state_.compare_exchange_strong(currentState, State::INACTIVE))
      {
        callback_();
      }
    }
  }

  EventLoop::Async::~Async() { internalAsync_.trigger(true); }

  void EventLoop::Async::trigger() { internalAsync_.trigger(false); }

} // namespace mdnscpp
