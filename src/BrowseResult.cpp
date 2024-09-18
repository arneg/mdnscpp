#include <mdnscpp/BrowseResult.h>

namespace mdnscpp
{
  const std::vector<TxtRecord> &BrowseResult::getTxtRecords() const
  {
    return txtRecords_;
  }

  const std::string &BrowseResult::getType() const { return type_; }
  const std::string &BrowseResult::getProtocol() const { return protocol_; }
  const std::string &BrowseResult::getName() const { return name_; }
  const std::string &BrowseResult::getDomain() const { return domain_; }
  const std::string &BrowseResult::getHostname() const { return hostname_; }
  const std::string &BrowseResult::getAddress() const { return address_; }

  size_t BrowseResult::getInterface() const { return interface_; }

  std::string BrowseResult::describe() const
  {
    std::string result = "BrowseResult(";

    result += "type: ";
    result += getType();

    result += ", protocol: ";
    result += getProtocol();

    result += ", name: ";
    result += getName();

    result += ", domain: ";
    result += getDomain();

    result += ", hostname: ";
    result += getHostname();

    result += ", address: ";
    result += getAddress();

    result += ", interface: ";
    result += std::to_string(getInterface());

    result += ")";

    return result;
  }

  BrowseResult::BrowseResult(std::vector<TxtRecord> txtRecords,
      std::string type, std::string protocol, std::string name,
      std::string domain, std::string hostname, std::string address,
      size_t interface)
      : txtRecords_(txtRecords), type_(type), protocol_(protocol), name_(name),
        domain_(domain), hostname_(hostname), address_(address),
        interface_(interface)
  {
  }
}; // namespace mdnscpp
