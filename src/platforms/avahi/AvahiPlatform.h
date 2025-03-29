#pragma once

#include <mdnscpp/EventLoop.h>
#include <mdnscpp/Platform.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

namespace mdnscpp
{
  class AvahiPlatform : public Platform,
                        public std::enable_shared_from_this<AvahiPlatform>
  {
  public:
    AvahiPlatform(EventLoop &loop);

    std::shared_ptr<Browser> createBrowser(const std::string &type,
        const std::string &protocol,
        std::function<void(const Browser &)> onResultsChanged,
        const std::string &domain = "", size_t interfaceIndex = 0) override;

    AvahiClient *getAvahiClient() const;

  private:
    struct AvahiPoll avahiPoll_;
    AvahiClient *avahiClient_;

    static void avahiClientCallback(
        AvahiClient *s, AvahiClientState state, void *userdata);
    static AvahiWatch *avahiPollWatchNew(const AvahiPoll *api, int fd,
        AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata);
    static void avahiPollWatchUpdate(AvahiWatch *w, AvahiWatchEvent event);
    static AvahiWatchEvent avahiPollWatchGetEvents(AvahiWatch *w);
    static void avahiPollWatchFree(AvahiWatch *w);
    static AvahiTimeout *avahiPollTimeoutNew(const AvahiPoll *api,
        const struct timeval *tv, AvahiTimeoutCallback callback,
        void *userdata);
    static void avahiPollTimeoutUpdate(
        AvahiTimeout *, const struct timeval *tv);
    static void avahiPollTimeoutFree(AvahiTimeout *t);
  };
} // namespace mdnscpp
