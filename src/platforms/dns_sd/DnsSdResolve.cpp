#include "DnsSdResolve.h"

#include "DnsSdPlatform.h"

#include <iostream>

#include <netdb.h>

namespace mdnscpp
{
  DnsSdResolve::DnsSdResolve(std::shared_ptr<DnsSdPlatform> platform,
      size_t interface, const std::string &name, const std::string &type,
      const std::string &domain)
      : DnsSdRef(platform, startResolve(interface, name, type, domain, this)),
        interface_(interface), name_(name), type_(type), domain_(domain)
  {
    std::cerr << describe() << std::endl;
  }

  DnsSdResolve::~DnsSdResolve() { std::cerr << "~" << describe() << std::endl; }

  std::string DnsSdResolve::describe() const
  {
    std::string result = "DnsSdResolve(";
    result += "if ";
    result += std::to_string(interface_);
    result += ", ";
    result += name_;
    result += ", ";
    result += type_;
    result += ", ";
    result += domain_;
    result += ")";
    return result;
  }

  DNSServiceRef DnsSdResolve::startResolve(size_t interface,
      const std::string &name, const std::string &type,
      const std::string &domain, void *context)
  {
    DNSServiceRef sdRef;

    auto error = DNSServiceResolve(&sdRef, 0, static_cast<uint32_t>(interface),
        name.c_str(), type.c_str(), domain.c_str(), resolveResultCallback,
        context);

    if (kDNSServiceErr_NoError != error)
      throw std::runtime_error("Failed.");

    return sdRef;
  }

  void DnsSdResolve::onResult(DNSServiceFlags flags, uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, const char *fullname,
      const char *hosttarget, uint16_t port, uint16_t txtLen,
      const unsigned char *txtRecord)
  {
    if (errorCode != kDNSServiceErr_NoError)
    {
      std::cerr << describe() << " failed with error " << errorCode
                << std::endl;
    }
    else
    {
      std::cerr << "Resolved " << fullname << " to " << hosttarget << " with "
                << txtLen << " bytes of txt records " << std::endl;

      getaddrinfo_ = std::make_shared<DnsSdGetAddrInfo>(
          platform_, interfaceIndex, hosttarget);

      close();
    }
  }

  void DnsSdResolve::resolveResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndex,
      DNSServiceErrorType errorCode, const char *fullname,
      const char *hosttarget, uint16_t port, uint16_t txtLen,
      const unsigned char *txtRecord, void *context)
  {
    reinterpret_cast<DnsSdResolve *>(context)->onResult(flags, interfaceIndex,
        errorCode, fullname, hosttarget, port, txtLen, txtRecord);
  }
} // namespace mdnscpp
