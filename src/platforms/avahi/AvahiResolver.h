#pragma once

#include <memory>
#include <string>

#include <mdnscpp/BrowseResult.h>
#include <mdnscpp/Types.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

namespace mdnscpp
{
  class AvahiBrowser;

  class AvahiResolver : public std::enable_shared_from_this<AvahiResolver>
  {
  public:
    AvahiResolver(std::shared_ptr<AvahiBrowser> browser,
        const std::string &name, const std::string &type,
        const std::string &domain, size_t interfaceIndex,
        IPProtocol ipProtocol);
    ~AvahiResolver();

    std::string describe() const;
    std::shared_ptr<AvahiBrowser> getBrowser() const;

  private:
    const std::weak_ptr<AvahiBrowser> browser_;
    const std::string name_;
    const std::string type_;
    const std::string domain_;
    const size_t interfaceIndex_;
    const IPProtocol ipProtocol_;

    std::shared_ptr<BrowseResult> result_;

    AvahiServiceResolver *avahiServiceResolver_ = nullptr;

    void resultCallback(AvahiIfIndex interface, AvahiProtocol protocol,
        AvahiResolverEvent event, const char *name, const char *type,
        const char *domain, const char *host_name, const AvahiAddress *a,
        uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags);

    static void avahiServiceResolverCallback(AvahiServiceResolver *r,
        AvahiIfIndex interface, AvahiProtocol protocol,
        AvahiResolverEvent event, const char *name, const char *type,
        const char *domain, const char *host_name, const AvahiAddress *a,
        uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags,
        void *userdata);
  };
} // namespace mdnscpp
