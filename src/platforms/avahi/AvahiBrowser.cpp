#include "AvahiBrowser.h"
#include "AvahiPlatform.h"
#include "AvahiUtils.h"

#include "../../debug.h"
#include "../../throw.h"

namespace mdnscpp
{
  AvahiBrowser::AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : Browser(type, protocol, onResultsChanged, domain, interfaceIndex,
            ipProtocol),
        platform_(platform)
  {
    MDNSCPP_INFO << describe() << " started" << MDNSCPP_ENDL;

    auto client = platform->getAvahiClient();

    std::string tmp = type + "." + protocol;
    avahiBrowser_ = avahi_service_browser_new(client,
        toAvahiInterfaceIndex(interfaceIndex), toAvahiProtocol(ipProtocol),
        tmp.c_str(), domain.c_str(), static_cast<AvahiLookupFlags>(0),
        avahiServiceBrowserCallback, this);

    MDNSCPP_INFO << "client: " << client << ", type: '" << tmp << "'"
                 << ", domain: '" << domain << "'"
                 << ", protocol: " << toAvahiProtocol(ipProtocol)
                 << MDNSCPP_ENDL;

    if (!avahiBrowser_)
      MDNSCPP_THROW(
          std::runtime_error, "Failed to create avahi service browser.");
  }

  AvahiBrowser::~AvahiBrowser()
  {
    MDNSCPP_INFO << describe() << " stopped" << MDNSCPP_ENDL;
    if (avahiBrowser_)
    {
      avahi_service_browser_free(avahiBrowser_);
    }
  }

  std::shared_ptr<Browser> AvahiBrowser::getSharedFromThis()
  {
    return shared_from_this();
  }

  AvahiClient *AvahiBrowser::getAvahiClient() const
  {
    return avahi_service_browser_get_client(avahiBrowser_);
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
    result += interfaceIndex_;
    result += ")";
    return result;
  }

  void AvahiBrowser::resultCallback(AvahiIfIndex interfaceIndex,
      AvahiProtocol protocol, AvahiBrowserEvent event, const char *name,
      const char *type, const char *domain, AvahiLookupResultFlags flags)
  {
    if (event == AVAHI_BROWSER_NEW || event == AVAHI_BROWSER_REMOVE)
    {
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

        resolvers_[key] = std::make_shared<AvahiResolver>(shared_from_this(),
            name, type, domain, interfaceIndex, fromAvahiProtocol(protocol));
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
    reinterpret_cast<AvahiBrowser *>(userdata)->resultCallback(
        interfaceIndex, protocol, event, name, type, domain, flags);
  }
} // namespace mdnscpp
