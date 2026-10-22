#include "AvahiBrowser.h"
#include "AvahiPlatform.h"
#include "AvahiUtils.h"

#include "../../debug.h"
#include "../../guardCallback.h"

#include <avahi-common/error.h>

namespace mdnscpp
{
  AvahiBrowser::AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : Browser(type, protocol, onResultsChanged, domain, interfaceIndex,
            ipProtocol, platform->getEventLoop()),
        platform_(platform)
  {
    MDNSCPP_INFO << describe() << " created" << MDNSCPP_ENDL;
  }

  AvahiBrowser::~AvahiBrowser()
  {
    platform_->removeBrowser(this);
    MDNSCPP_INFO << describe() << " destroyed" << MDNSCPP_ENDL;
    pause();
  }

  std::shared_ptr<Browser> AvahiBrowser::getSharedFromThis()
  {
    return shared_from_this();
  }

  AvahiClient *AvahiBrowser::getAvahiClient() const
  {
    return platform_->getAvahiClient();
  }

  std::string AvahiBrowser::describe() const
  {
    std::string result = "AvahiBrowse(";
    result += type_;
    result += ", ";
    result += protocol_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += std::to_string(interfaceIndex_);
    result += ")";
    return result;
  }

  void AvahiBrowser::pause()
  {
    resolvers_.clear();

    if (avahiBrowser_)
    {
      avahi_service_browser_free(avahiBrowser_);
      avahiBrowser_ = nullptr;
    }

    MDNSCPP_INFO << describe() << " stopped" << MDNSCPP_ENDL;
  }

  bool AvahiBrowser::unpause()
  {
    if (avahiBrowser_)
      return true;

    AvahiClient *client = getAvahiClient();
    if (!client)
    {
      MDNSCPP_ERROR << describe() << " cannot start: no Avahi client."
                    << MDNSCPP_ENDL;
      return false;
    }

    std::string tmp = getType() + "." + getProtocol();

    avahiBrowser_ = avahi_service_browser_new(client,
        toAvahiInterfaceIndex(getInterface()), toAvahiProtocol(getIPProtocol()),
        tmp.c_str(), getDomain().c_str(), static_cast<AvahiLookupFlags>(0),
        avahiServiceBrowserCallback, this);

    MDNSCPP_INFO << ", type: '" << tmp << "'"
                 << ", domain: '" << getDomain() << "'"
                 << ", protocol: " << toAvahiProtocol(getIPProtocol())
                 << MDNSCPP_ENDL;

    if (!avahiBrowser_)
    {
      MDNSCPP_ERROR << describe() << " failed to create avahi service browser: "
                    << avahi_strerror(avahi_client_errno(client))
                    << MDNSCPP_ENDL;
      return false;
    }

    MDNSCPP_INFO << describe() << " started" << MDNSCPP_ENDL;
    return true;
  }

  void AvahiBrowser::resultCallback(AvahiIfIndex interfaceIndex,
      AvahiProtocol protocol, AvahiBrowserEvent event, const char *name,
      const char *type, const char *domain, AvahiLookupResultFlags flags)
  {
    if (event == AVAHI_BROWSER_NEW || event == AVAHI_BROWSER_REMOVE)
    {
      if (!name || !type || !domain)
      {
        MDNSCPP_ERROR << describe()
                      << " ignoring browse event with a missing name, type or "
                         "domain."
                      << MDNSCPP_ENDL;
        return;
      }

      std::string key;

      key.reserve(32);

      key += std::to_string(interfaceIndex);
      key += ",";
      key += type;
      key += domain;
      key += ",";
      key += protocol == AVAHI_PROTO_INET6 ? "ipv6" : "ipv4";
      key += ",";
      key += name;

      if (event == AVAHI_BROWSER_NEW)
      {
        MDNSCPP_INFO << describe() << " NEW " << key << MDNSCPP_ENDL;

        // NEW can happen several times due to data from the cache.
        if (resolvers_.find(key) != resolvers_.end())
          return;

        // start() after make_shared so that a synchronous Avahi callback can
        // use weak_from_this(). A failed start is skipped: the service will
        // not appear in the results.
        auto resolver = std::make_shared<AvahiResolver>(shared_from_this(),
            name, type, domain, interfaceIndex, fromAvahiProtocol(protocol));
        if (!resolver->start())
          return;
        resolvers_[key] = std::move(resolver);
      }
      else
      {
        MDNSCPP_INFO << describe() << " REMOVE " << key << MDNSCPP_ENDL;
        resolvers_.erase(key);
      }
    }
  }

  void AvahiBrowser::avahiServiceBrowserCallback(AvahiServiceBrowser *b,
      AvahiIfIndex interfaceIndex, AvahiProtocol protocol,
      AvahiBrowserEvent event, const char *name, const char *type,
      const char *domain, AvahiLookupResultFlags flags, void *userdata)
  {
    guardCallback("AvahiBrowser::avahiServiceBrowserCallback", [&] {
      reinterpret_cast<AvahiBrowser *>(userdata)->resultCallback(
          interfaceIndex, protocol, event, name, type, domain, flags);
    });
  }
} // namespace mdnscpp
