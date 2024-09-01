#include "AvahiPlatform.h"

namespace mdnscpp
{
  AvahiPlatform::AvahiPlatform(
      std::function<void(int)> watchFd, std::function<void(int)> unwatchFd)
      : Platform(watchFd, unwatchFd)
  {
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
      throw std::runtime_error("avahi_client_new failed.");
  }

  std::shared_ptr<Browser> AvahiPlatform::createBrowser(const std::string &type,
      const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interface)
  {
    return nullptr;
  }

  void AvahiPlatform::process(int fd) {}

  void AvahiPlatform::avahiClientCallback(
      AvahiClient *s, AvahiClientState state, void *userdata)
  {
  }

  AvahiWatch *AvahiPlatform::avahiPollWatchNew(const AvahiPoll *api, int fd,
      AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata)
  {
  }
  void AvahiPlatform::avahiPollWatchUpdate(AvahiWatch *w, AvahiWatchEvent event)
  {
  }
  AvahiWatchEvent AvahiPlatform::avahiPollWatchGetEvents(AvahiWatch *w) {}
  void AvahiPlatform::avahiPollWatchFree(AvahiWatch *w) {}
  AvahiTimeout *AvahiPlatform::avahiPollTimeoutNew(const AvahiPoll *api,
      const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata)
  {
  }
  void AvahiPlatform::avahiPollTimeoutUpdate(
      AvahiTimeout *, const struct timeval *tv)
  {
  }
  void AvahiPlatform::avahiPollTimeoutFree(AvahiTimeout *t) {}
} // namespace mdnscpp
