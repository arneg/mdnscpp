#pragma once

#include <string>
#include <utility>
#include <vector>

#include "TxtRecord.h"
#include "Types.h"

namespace mdnscpp
{
  class BrowseResult
  {
  public:
    /**
     * The list of txt records.
     */
    const std::vector<TxtRecord> &getTxtRecords() const;

    /** The type, e.g. `_http`. */
    const std::string &getType() const;

    /** The protocol, e.g. `_tcp`. */
    const std::string &getProtocol() const;

    /** The service name. */
    const std::string &getName() const;

    /** 
     * Returns the full service name, e.g. example._http._tcp.local. In usual situations this
     * will uniquely identity a service.
     */
    const std::string &getFullname() const;

    /** The domain, e.g. `local`. */
    const std::string &getDomain() const;

    /** The hostname of the service. */
    const std::string &getHostname() const;

    /**
     * The IP adress of the service. This can be either
     * a IPv4 or IPv6 address.
     */
    const std::string &getAddress() const;

    std::string describe() const;

    /** The interface index this service was found on. */
    size_t getInterface() const;

    IPProtocol getIPProtocol() const;

    bool operator==(const BrowseResult &other) const;
    bool operator!=(const BrowseResult &other) const;

    BrowseResult(std::vector<TxtRecord> txtRecords, std::string type,
        std::string protocol, std::string name, std::string domain,
        std::string hostname, std::string address, size_t interfaceIndex,
        IPProtocol ipProtocol);

    /**
     * Returns true if this result is less than (i.e. ordered before) the result.
     */
    bool operator<(const BrowseResult &b) const;

  private:
    std::vector<TxtRecord> txtRecords_;
    std::string type_;
    std::string protocol_;
    std::string name_;
    std::string domain_;
    std::string hostname_;
    std::string address_;
    std::string fullname_;
    size_t interfaceIndex_;
    IPProtocol ipProtocol_;
  };
} // namespace mdnscpp
