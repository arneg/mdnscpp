#include "AvahiBrowser.h"
#include "AvahiPlatform.h"

#include "../../debug.h"
#include "../../throw.h"

namespace mdnscpp
{
  static AvahiProtocol toAvahiProtocol(IPProtocol ipProtocol)
  {
    switch (ipProtocol)
    {
    case IPProtocol::IPv4:
      return AVAHI_PROTO_INET;
    case IPProtocol::IPv6:
      return AVAHI_PROTO_INET6;
    default:
      return AVAHI_PROTO_UNSPEC;
    }
  }

  AvahiBrowser::AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : Browser(type, protocol, onResultsChanged, domain, interfaceIndex,
            ipProtocol)
  {
    std::string tmp = type + "." + protocol;
    // TODO: translate interface index
    avahiBrowser_ = avahi_service_browser_new(platform->getAvahiClient(),
        AVAHI_IF_UNSPEC, toAvahiProtocol(ipProtocol), tmp.c_str(),
        domain.size() ? domain.c_str() : NULL, static_cast<AvahiLookupFlags>(0),
        avahiServiceBrowserCallback, this);

    if (!avahiBrowser_)
      MDNSCPP_THROW(
          std::runtime_error, "Failed to create avahi service browser.");

    MDNSCPP_INFO << describe() << " started" << MDNSCPP_ENDL;
  }

  std::shared_ptr<Browser> AvahiBrowser::getSharedFromThis()
  {
    return shared_from_this();
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
    MDNSCPP_INFO << describe() << " avahiServiceBrowserCallback("
                 << (name ? name : "nil") << ", " << (type ? type : "nil")
                 << ", " << (domain ? domain : "nil") << ", " << interfaceIndex
                 << ")" << MDNSCPP_ENDL;
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
