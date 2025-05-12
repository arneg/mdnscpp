#pragma once

#include <vector>

#include <mdnscpp/EventLoop.h>
#include <mdnscpp/Platform.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

namespace mdnscpp
{
  class AvahiBrowser;

  class AvahiPlatform : public Platform,
                        public std::enable_shared_from_this<AvahiPlatform>
  {
    friend AvahiBrowser;
  public:
    AvahiPlatform(EventLoop &loop);
    ~AvahiPlatform();

    // TODO: Do we need the default arguments here?
    std::shared_ptr<Browser> createBrowser(const std::string &type,
        const std::string &protocol,
        Browser::ResultsChangedCallback onResultsChanged,
        const std::string &domain = "", size_t interfaceIndex = 0,
        IPProtocol ipProtocol = IPProtocol::Both) override;

    AvahiClient *getAvahiClient() const;

  private:
    // We need to keep this list in order to call pause()
    // and unpause()
    std::vector<AvahiBrowser*> browsers_;
    struct AvahiPoll avahiPoll_;
    AvahiClient *avahiClient_;
    bool paused_ = true;

    void startClient();
    void stopClient();
    void pause();
    void unpause();

    void removeBrowser(AvahiBrowser *browser);

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
