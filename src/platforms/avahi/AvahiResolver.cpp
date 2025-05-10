#include "AvahiResolver.h"
#include "AvahiBrowser.h"
#include "AvahiPlatform.h"
#include "AvahiUtils.h"

#include "../../debug.h"
#include "../../throw.h"

#include <avahi-common/malloc.h>

namespace mdnscpp
{
  AvahiResolver::AvahiResolver(std::shared_ptr<AvahiBrowser> browser,
      const std::string &name, const std::string &type,
      const std::string &domain, size_t interfaceIndex, IPProtocol ipProtocol)
      : browser_(browser), name_(name), type_(type), domain_(domain),
        interfaceIndex_(interfaceIndex), ipProtocol_(ipProtocol)
  {
    avahiServiceResolver_ = avahi_service_resolver_new(
        browser->getAvahiClient(), interfaceIndex, toAvahiProtocol(ipProtocol),
        name.c_str(), type.c_str(), domain.c_str(),
        toAvahiProtocol(browser->getIPProtocol()),
        static_cast<AvahiLookupFlags>(0), avahiServiceResolverCallback, this);

    if (!avahiServiceResolver_)
      MDNSCPP_THROW(
          std::runtime_error, "Failed to create avahi service resolve.");
  }

  AvahiResolver::~AvahiResolver()
  {
    if (avahiServiceResolver_)
    {
      avahi_service_resolver_free(avahiServiceResolver_);
    }

    auto browser = getBrowser();

    if (browser && result_)
    {
      browser->removeResult(result_);
    }
  }

  std::string AvahiResolver::describe() const
  {
    std::string result = "AvahiResolver(";
    result += name_;
    result += ", ";
    result += type_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += interfaceIndex_;
    result += ")";
    return result;
  }

  std::shared_ptr<AvahiBrowser> AvahiResolver::getBrowser() const
  {
    return browser_.lock();
  }

  void AvahiResolver::resultCallback(AvahiIfIndex interface,
      AvahiProtocol protocol, AvahiResolverEvent event, const char *name,
      const char *type, const char *domain, const char *host_name,
      const AvahiAddress *a, uint16_t port, AvahiStringList *txt,
      AvahiLookupResultFlags flags)
  {
    MDNSCPP_INFO << describe() << " result(" << host_name << ")"
                 << MDNSCPP_ENDL;

    std::shared_ptr<BrowseResult> result;
    auto browser = getBrowser();

    // We are about to be removed?
    if (!browser)
      return;

    if (event == AVAHI_RESOLVER_FOUND)
    {
      std::vector<TxtRecord> txtRecords;

      if (txt)
      {
        txtRecords.reserve(avahi_string_list_get_size(txt));

        for (AvahiStringList *it = txt; it; it = avahi_string_list_get_next(it))
        {
          char *key = nullptr, *value = nullptr;
          avahi_string_list_get_pair(it, &key, &value, nullptr);

          MDNSCPP_INFO << "key: " << key << ", value: " << value
                       << MDNSCPP_ENDL;

          TxtRecord entry;

          if (key)
          {
            entry.key = key;
            avahi_free(key);

            if (value)
            {
              entry.value.emplace(value);
              avahi_free(value);
            }

            txtRecords.push_back(std::move(entry));
          }
        }
      }

      result = std::make_shared<BrowseResult>(txtRecords, browser->getType(),
          browser->getProtocol(), name_, domain_, host_name,
          fromAvahiAddress(a), interfaceIndex_, fromAvahiProtocol(protocol),
          port);
    }

    if (result_)
    {
      browser->removeResult(result_);
    }

    std::swap(result_, result);

    if (result_)
    {
      browser->insertResult(result_);
    }
  }

  void AvahiResolver::avahiServiceResolverCallback(AvahiServiceResolver *r,
      AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
      const char *name, const char *type, const char *domain,
      const char *host_name, const AvahiAddress *a, uint16_t port,
      AvahiStringList *txt, AvahiLookupResultFlags flags, void *userdata)
  {
    std::shared_ptr<AvahiResolver> self =
        reinterpret_cast<AvahiResolver *>(userdata)->shared_from_this();
    self->resultCallback(interface, protocol, event, name, type, domain,
        host_name, a, port, txt, flags);
  }
} // namespace mdnscpp
