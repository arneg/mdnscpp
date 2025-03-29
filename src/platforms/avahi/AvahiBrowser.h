#pragma once

#include <avahi-client/lookup.h>

#include <mdnscpp/Browser.h>
#include <memory>
#include <string>

namespace mdnscpp
{
  class AvahiPlatform;

  class AvahiBrowser : public Browser
  {
  public:
    AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
        const std::string &type, const std::string &protocol,
        std::function<void(const Browser &)> onResultsChanged,
        const std::string &domain, size_t interfaceIndex);

    std::string describe() const;

  private:
    AvahiServiceBrowser *avahiBrowser_;

    void resultCallback(AvahiIfIndex interfaceIndex, AvahiProtocol protocol,
        AvahiBrowserEvent event, const char *name, const char *type,
        const char *domain, AvahiLookupResultFlags flags);

    static void avahiServiceBrowserCallback(AvahiServiceBrowser *b,
        AvahiIfIndex interfaceIndex, AvahiProtocol protocol,
        AvahiBrowserEvent event, const char *name, const char *type,
        const char *domain, AvahiLookupResultFlags flags, void *userdata);
  };
} // namespace mdnscpp
