#include "AvahiBrowser.h"
#include "AvahiPlatform.h"

#include "../../debug.h"
#include "../../throw.h"

namespace mdnscpp
{
  AvahiBrowser::AvahiBrowser(std::shared_ptr<AvahiPlatform> platform,
      const std::string &type, const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interfaceIndex)
      : Browser(type, protocol, onResultsChanged, domain, interfaceIndex)
  {
    std::string tmp = type + "." + protocol;
    avahiBrowser_ = avahi_service_browser_new(platform->getAvahiClient(),
        AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, tmp.c_str(), NULL,
        static_cast<AvahiLookupFlags>(0), avahiServiceBrowserCallback, this);

    if (!avahiBrowser_)
      MDNSCPP_THROW(
          std::runtime_error, "Failed to create avahi service browser.");

    MDNSCPP_INFO << describe() << " started" << MDNSCPP_ENDL;
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
