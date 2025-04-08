#include "AvahiPlatform.h"
#include "AvahiBrowser.h"

#include "../../throw.h"

#include "../../debug.h"

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
      result.setRead();
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
      state = EventLoop::TimeoutAbsolute{*tv};
    }

    return state;
  }

  AvahiPlatform::AvahiPlatform(EventLoop &loop) : Platform(loop)
  {
    MDNSCPP_INFO << "this " << (void *)(this) << MDNSCPP_ENDL;
    MDNSCPP_INFO << "loop " << (void *)(&loop) << MDNSCPP_ENDL;
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

    avahiClient_ = avahi_client_new(
        &avahiPoll_, AVAHI_CLIENT_NO_FAIL, avahiClientCallback, this, &err);

    if (!avahiClient_)
      MDNSCPP_THROW(std::runtime_error, "avahi_client_new failed.");
  }

  std::shared_ptr<Browser> AvahiPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interfaceIndex)
  {
    return std::make_shared<AvahiBrowser>(shared_from_this(), type, protocol,
        onResultsChanged, domain, interfaceIndex);
  }

  AvahiClient *AvahiPlatform::getAvahiClient() const { return avahiClient_; }

  void AvahiPlatform::avahiClientCallback(
      AvahiClient *s, AvahiClientState state, void *userdata)
  {
  }

  AvahiWatch *AvahiPlatform::avahiPollWatchNew(const AvahiPoll *api, int fd,
      AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata)
  {
    AvahiPlatform *self = reinterpret_cast<AvahiPlatform *>(api->userdata);
    AvahiWatch *w = new AvahiWatch();

    w->watch = self->loop_.createWatch(
        fd, fromAvahiEvent(event), [=](EventLoop::EventType events) {
          callback(w, fd, toAvahiEvent(events), userdata);
        });

    return w;
  }

  void AvahiPlatform::avahiPollWatchUpdate(AvahiWatch *w, AvahiWatchEvent event)
  {
    w->watch->update(fromAvahiEvent(event));
  }

  AvahiWatchEvent AvahiPlatform::avahiPollWatchGetEvents(AvahiWatch *w)
  {
    return toAvahiEvent(w->watch->getRequestedEvents());
  }

  void AvahiPlatform::avahiPollWatchFree(AvahiWatch *w) { delete w; }

  AvahiTimeout *AvahiPlatform::avahiPollTimeoutNew(const AvahiPoll *api,
      const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata)
  {
    AvahiPlatform *self = reinterpret_cast<AvahiPlatform *>(api->userdata);
    AvahiTimeout *t = new AvahiTimeout();

    MDNSCPP_INFO << "self " << (void *)(self) << MDNSCPP_ENDL;
    MDNSCPP_INFO << "loop " << (void *)(&self->loop_) << MDNSCPP_ENDL;

    t->timeout = self->loop_.createTimeout(stateFromTimeval(tv),
        [=](EventLoop::Timeout &) { callback(t, userdata); });

    return t;
  }

  void AvahiPlatform::avahiPollTimeoutUpdate(
      AvahiTimeout *t, const struct timeval *tv)
  {
    t->timeout->update(stateFromTimeval(tv));
  }

  void AvahiPlatform::avahiPollTimeoutFree(AvahiTimeout *t) { delete t; }
} // namespace mdnscpp
