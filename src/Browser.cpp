#include <mdnscpp/Browser.h>

namespace mdnscpp
{
  Browser::Browser(const std::string &type, const std::string &protocol,
      std::function<void(const Browser &)> onResultsChanged,
      const std::string &domain, size_t interface, IPProtocol ipProtocol)
      : type_(type), protocol_(protocol), onResultsChanged_(onResultsChanged),
        domain_(domain), interface_(interface), ipProtocol_(ipProtocol)
  {
  }

  const std::string &Browser::getType() const { return type_; }

  const std::string &Browser::getProtocol() const { return protocol_; }

  const std::string &Browser::getDomain() const { return domain_; }

  size_t Browser::getInterface() const { return interface_; }

  IPProtocol Browser::getIPProtocol() const { return ipProtocol_; }
} // namespace mdnscpp
