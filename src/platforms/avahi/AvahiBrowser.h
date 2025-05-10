#pragma once

#include "AvahiResolver.h"

#include <avahi-client/lookup.h>

#include <mdnscpp/Browser.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace mdnscpp
{
  class AvahiPlatform;

  class AvahiBrowser : public Browser,
                       public std::enable_shared_from_this<AvahiBrowser>
  {
  public:
    AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
        const std::string &type, const std::string &protocol,
        ResultsChangedCallback onResultsChanged, const std::string &domain,
        size_t interfaceIndex, IPProtocol ipProtocol);
    ~AvahiBrowser();

    std::string describe() const;

    AvahiClient *getAvahiClient() const;

  protected:
    std::shared_ptr<Browser> getSharedFromThis() override;

  private:
    std::shared_ptr<AvahiPlatform> platform_;
    AvahiServiceBrowser *avahiBrowser_;
    std::unordered_map<std::string, std::shared_ptr<AvahiResolver>> resolvers_;

    void resultCallback(AvahiIfIndex interfaceIndex, AvahiProtocol protocol,
        AvahiBrowserEvent event, const char *name, const char *type,
        const char *domain, AvahiLookupResultFlags flags);

    static void avahiServiceBrowserCallback(AvahiServiceBrowser *b,
        AvahiIfIndex interfaceIndex, AvahiProtocol protocol,
        AvahiBrowserEvent event, const char *name, const char *type,
        const char *domain, AvahiLookupResultFlags flags, void *userdata);
  };
} // namespace mdnscpp
