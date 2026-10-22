#include "AvahiPlatform.h"
#include "AvahiBrowser.h"

#include <algorithm>
#include <string>

#include <avahi-common/error.h>
#include <mdnscpp/Error.h>

#include "../../debug.h"
#include "../../guardCallback.h"
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
  static std::string failedToConnectMessage(int err)
  {
    return std::string("Failed to connect to the Avahi daemon: ") +
           avahi_strerror(err);
  }

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

    if (!startClient())
      MDNSCPP_THROW(PlatformError, failedToConnectMessage(lastClientError_));
  }

  AvahiPlatform::~AvahiPlatform()
  {
    MDNSCPP_ASSERT(browsers_.size() == 0);
    MDNSCPP_INFO << "~AvahiPlatform(" << this << ")" << MDNSCPP_ENDL;
    stopClient();
  }

  std::shared_ptr<Browser> AvahiPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      Browser::ResultsChangedCallback onResultsChanged,
      const std::string &domain, size_t interfaceIndex, IPProtocol ipProtocol)
  {
    if (!avahiClient_)
      MDNSCPP_THROW(PlatformError, failedToConnectMessage(lastClientError_));

    auto browser = std::make_shared<AvahiBrowser>(shared_from_this(), type,
        protocol, onResultsChanged, domain, interfaceIndex, ipProtocol);

    // Synchronous path: a failure here is reported to the caller.
    if (!paused_ && !browser->unpause())
    {
      MDNSCPP_THROW(PlatformError,
          std::string("Failed to create avahi service browser: ") +
              avahi_strerror(avahi_client_errno(avahiClient_)));
    }

    browsers_.push_back(browser.get());

    return browser;
  }

  AvahiClient *AvahiPlatform::getAvahiClient() const { return avahiClient_; }

  bool AvahiPlatform::startClient()
  {
    int err = AVAHI_OK;

    avahiClient_ = avahi_client_new(
        &avahiPoll_, AVAHI_CLIENT_NO_FAIL, avahiClientCallback, this, &err);

    if (!avahiClient_)
    {
      // AVAHI_CLIENT_NO_FAIL only covers a missing Avahi daemon. If the
      // system D-Bus cannot be reached at all (e.g. in a container) the
      // client cannot be created. Do not throw: this is also called from
      // the Avahi client callback (see pause()).
      lastClientError_ = err;
      MDNSCPP_ERROR << failedToConnectMessage(err) << MDNSCPP_ENDL;
      return false;
    }

    lastClientError_ = AVAHI_OK;
    return true;
  }

  void AvahiPlatform::stopClient()
  {
    if (avahiClient_)
    {
      avahi_client_free(avahiClient_);
      avahiClient_ = nullptr;
    }
  }

  void AvahiPlatform::pause()
  {
    if (paused_)
      return;
    paused_ = true;
    for (AvahiBrowser *browser : browsers_)
    {
      browser->pause();
    }
    stopClient();
    // Restarting can fail (D-Bus gone). Log and leave the client null;
    // createBrowser() will surface this as a PlatformError. Browsers stay
    // paused.
    startClient();
  }

  void AvahiPlatform::unpause()
  {
    if (!paused_)
      return;
    if (!avahiClient_)
    {
      MDNSCPP_ERROR << "AvahiPlatform::unpause(): no Avahi client."
                    << MDNSCPP_ENDL;
      return;
    }
    paused_ = false;
    for (AvahiBrowser *browser : browsers_)
    {
      // Failure is logged inside unpause(). Keep going for the others.
      browser->unpause();
    }
  }

  void AvahiPlatform::removeBrowser(AvahiBrowser *browser)
  {
    auto it = std::remove(browsers_.begin(), browsers_.end(), browser);
    browsers_.erase(it, browsers_.end());
  }

  void AvahiPlatform::avahiClientCallback(
      AvahiClient *s, AvahiClientState state, void *userdata)
  {
    guardCallback("AvahiPlatform::avahiClientCallback", [&] {
      MDNSCPP_INFO << "avahiClientCallback(" << state << ")" << MDNSCPP_ENDL;
      AvahiPlatform *platform = reinterpret_cast<AvahiPlatform *>(userdata);
      switch (state)
      {
      case AVAHI_CLIENT_S_RUNNING:
        platform->unpause();
        break;
      default:
        platform->pause();
        break;
      }
    });
  }

  AvahiWatch *AvahiPlatform::avahiPollWatchNew(const AvahiPoll *api, int fd,
      AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata)
  {
    return guardCallback(
        "AvahiPlatform::avahiPollWatchNew",
        [&] {
          AvahiPlatform *self =
              reinterpret_cast<AvahiPlatform *>(api->userdata);
          AvahiWatch *w = new AvahiWatch();

          MDNSCPP_INFO << "watch_new(" << api << ", " << fd << ", " << ", "
                       << event << ", " << userdata << ")" << MDNSCPP_ENDL;

          w->watch = self->loop_.createWatch(
              fd, fromAvahiEvent(event), [=](EventLoop::EventType events) {
                auto event = toAvahiEvent(events);
                MDNSCPP_INFO << "watch triggered " << api << ", " << fd << ", "
                             << ", " << event << ", " << userdata << ")"
                             << MDNSCPP_ENDL;
                callback(w, fd, event, userdata);
              });

          return w;
        },
        static_cast<AvahiWatch *>(nullptr));
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
    return guardCallback(
        "AvahiPlatform::avahiPollTimeoutNew",
        [&] {
          AvahiPlatform *self =
              reinterpret_cast<AvahiPlatform *>(api->userdata);
          AvahiTimeout *t = new AvahiTimeout();

          MDNSCPP_INFO << "timeout_new(" << (tv ? timevalToMs(*tv) : size_t{0})
                       << ") = " << t << MDNSCPP_ENDL;

          t->timeout = self->loop_.createTimeout(stateFromTimeval(tv),
              [=](EventLoop::Timeout &) { callback(t, userdata); });

          return t;
        },
        static_cast<AvahiTimeout *>(nullptr));
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
