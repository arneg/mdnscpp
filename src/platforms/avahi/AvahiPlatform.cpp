#include "AvahiPlatform.h"
#include "AvahiBrowser.h"

#include "../../debug.h"
#include "../../throw.h"
#include "../../timeval.h"

struct AvahiWatch
{
  std::shared_ptr<mdnscpp::EventLoop::Watch> watch;
};

struct AvahiTimeout
{
  std::shared_ptr<mdnscpp::EventLoop::Timeout> timeout;
};

namespace mdnscpp
{
  static EventLoop::EventType fromAvahiEvent(AvahiWatchEvent event)
  {
    EventLoop::EventType result;

    int tmp = static_cast<int>(event);

    if (tmp & AVAHI_WATCH_IN)
    {
      MDNSCPP_INFO << "activating read" << MDNSCPP_ENDL;
      result.setRead();
    }
    if (tmp & AVAHI_WATCH_OUT)
      result.setWrite();
    if (tmp & AVAHI_WATCH_ERR)
      result.setError();
    if (tmp & AVAHI_WATCH_HUP)
      result.setDisconnect();

    return result;
  }

  static AvahiWatchEvent toAvahiEvent(EventLoop::EventType event)
  {
    int result = 0;

    if (event.hasRead())
      result |= AVAHI_WATCH_IN;
    if (event.hasWrite())
      result |= AVAHI_WATCH_OUT;
    if (event.hasError())
      result |= AVAHI_WATCH_ERR;
    if (event.hasDisconnect())
      result |= AVAHI_WATCH_HUP;

    return static_cast<AvahiWatchEvent>(result);
  }

  static EventLoop::TimeoutState stateFromTimeval(const struct timeval *tv)
  {
    EventLoop::TimeoutState state;

    if (tv)
    {
      if (tv->tv_sec || tv->tv_usec)
      {
        struct timeval now = getNow();

        struct timeval diff{0, 0};

        if (isBefore(now, *tv))
          diff = subTime(*tv, now);

        state = EventLoop::TimeoutRelative{diff};
      }
      else
      {
        state = EventLoop::TimeoutRelative{*tv};
      }
    }
    else
    {
      state = EventLoop::TimeoutDisabled{};
    }

    return state;
  }

  AvahiPlatform::AvahiPlatform(EventLoop &loop) : Platform(loop)
  {
    MDNSCPP_INFO << "AvahiPlatform(" << this << ")" << MDNSCPP_ENDL;
    avahiPoll_ = {
        .userdata = this,
        .watch_new = avahiPollWatchNew,
        .watch_update = avahiPollWatchUpdate,
        .watch_get_events = avahiPollWatchGetEvents,
        .watch_free = avahiPollWatchFree,
        .timeout_new = avahiPollTimeoutNew,
        .timeout_update = avahiPollTimeoutUpdate,
        .timeout_free = avahiPollTimeoutFree,
    };

    int err;

    avahiClient_ = avahi_client_new(&avahiPoll_,
        static_cast<AvahiClientFlags>(0) /*AVAHI_CLIENT_NO_FAIL*/, NULL, this,
        &err);

    if (!avahiClient_)
      MDNSCPP_THROW(std::runtime_error, "avahi_client_new failed.");
  }

  AvahiPlatform::~AvahiPlatform()
  {
    MDNSCPP_INFO << "~AvahiPlatform(" << this << ")" << MDNSCPP_ENDL;
    if (avahiClient_)
    {
      avahi_client_free(avahiClient_);
    }
  }

  std::shared_ptr<Browser> AvahiPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      Browser::ResultsChangedCallback onResultsChanged,
      const std::string &domain, size_t interfaceIndex, IPProtocol ipProtocol)
  {
    return std::make_shared<AvahiBrowser>(shared_from_this(), type, protocol,
        onResultsChanged, domain, interfaceIndex, ipProtocol);
  }

  AvahiClient *AvahiPlatform::getAvahiClient() const { return avahiClient_; }

  void AvahiPlatform::avahiClientCallback(
      AvahiClient *s, AvahiClientState state, void *userdata)
  {
    MDNSCPP_INFO << "avahiClientCallback(" << state << ")" << MDNSCPP_ENDL;
  }

  AvahiWatch *AvahiPlatform::avahiPollWatchNew(const AvahiPoll *api, int fd,
      AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata)
  {
    AvahiPlatform *self = reinterpret_cast<AvahiPlatform *>(api->userdata);
    AvahiWatch *w = new AvahiWatch();

    MDNSCPP_INFO << "watch_new(" << api << ", " << fd << ", " << ", " << event
                 << ", " << userdata << ")" << MDNSCPP_ENDL;

    w->watch = self->loop_.createWatch(
        fd, fromAvahiEvent(event), [=](EventLoop::EventType events) {
          auto event = toAvahiEvent(events);
          MDNSCPP_INFO << "watch triggered " << api << ", " << fd << ", "
                       << ", " << event << ", " << userdata << ")"
                       << MDNSCPP_ENDL;
          callback(w, fd, event, userdata);
        });

    return w;
  }

  void AvahiPlatform::avahiPollWatchUpdate(AvahiWatch *w, AvahiWatchEvent event)
  {
    MDNSCPP_INFO << "watch_update(" << event << ")" << MDNSCPP_ENDL;
    w->watch->update(fromAvahiEvent(event));
  }

  AvahiWatchEvent AvahiPlatform::avahiPollWatchGetEvents(AvahiWatch *w)
  {
    auto event = toAvahiEvent(w->watch->getRequestedEvents());
    MDNSCPP_INFO << "watch_get_events(" << event << ")" << MDNSCPP_ENDL;
    return event;
  }

  void AvahiPlatform::avahiPollWatchFree(AvahiWatch *w)
  {
    MDNSCPP_INFO << "watch_free(" << w << ")" << MDNSCPP_ENDL;
    delete w;
  }

  AvahiTimeout *AvahiPlatform::avahiPollTimeoutNew(const AvahiPoll *api,
      const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata)
  {
    AvahiPlatform *self = reinterpret_cast<AvahiPlatform *>(api->userdata);
    AvahiTimeout *t = new AvahiTimeout();

    MDNSCPP_INFO << "timeout_new(" << (tv ? timevalToMs(*tv) : size_t{0})
                 << ") = " << t << MDNSCPP_ENDL;

    t->timeout = self->loop_.createTimeout(stateFromTimeval(tv),
        [=](EventLoop::Timeout &) { callback(t, userdata); });

    return t;
  }

  void AvahiPlatform::avahiPollTimeoutUpdate(
      AvahiTimeout *t, const struct timeval *tv)
  {
    MDNSCPP_INFO << "timeout_update(" << t << ", "
                 << (tv ? timevalToMs(*tv) : size_t{0}) << ")" << MDNSCPP_ENDL;
    t->timeout->update(stateFromTimeval(tv));
  }

  void AvahiPlatform::avahiPollTimeoutFree(AvahiTimeout *t)
  {
    MDNSCPP_INFO << "timeout_free(" << t << ")" << MDNSCPP_ENDL;
    delete t;
  }
} // namespace mdnscpp
