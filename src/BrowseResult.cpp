#include <mdnscpp/BrowseResult.h>

#include <tuple>

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
  const std::string &BrowseResult::getFullname() const { return fullname_; }

  size_t BrowseResult::getInterface() const { return interface_; }
  IPProtocol BrowseResult::getIPProtocol() const { return ipProtocol_; }

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

    result += ", ipProtocol: ";
    result += describeIPProtocol(getIPProtocol());

    result += ", txt: [";
    for (size_t i = 0; i < txtRecords_.size(); i++)
    {
      const auto &record = txtRecords_[i];
      if (i)
        result += ", ";
      result += record.key;
      result += "=";
      if (record.value)
      {
        result += *record.value;
      }
      else
      {
        result += "(nul)";
      }
    }
    result += "]";

    result += ")";

    return result;
  }

  bool BrowseResult::operator<(const BrowseResult &b) const
  {
    return std::tie(fullname_, hostname_, interface_, ipProtocol_, address_) <
           std::tie(b.fullname_, b.hostname_, b.interface_, b.ipProtocol_,
               b.address_);
  }

  BrowseResult::BrowseResult(std::vector<TxtRecord> txtRecords,
      std::string type, std::string protocol, std::string name,
      std::string domain, std::string hostname, std::string address,
      size_t interface, IPProtocol ipProtocol)
      : txtRecords_(std::move(txtRecords)), type_(std::move(type)),
        protocol_(std::move(protocol)), name_(std::move(name)),
        domain_(std::move(domain)), hostname_(std::move(hostname)),
        address_(std::move(address)), interface_(interface),
        ipProtocol_(ipProtocol)
  {
    fullname_ = name_ + "." + type_ + "." + protocol_ + "." + domain;
  }
}; // namespace mdnscpp
