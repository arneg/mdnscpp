#include "DnsSdResolve.h"

#include "../../throw.h"
#include "DnsSdBrowser.h"
#include "DnsSdPlatform.h"

#include <iostream>

#include <netdb.h>

namespace mdnscpp
{
  static std::vector<TxtRecord> parseTxtRecords(
      const unsigned char *src, uint16_t length)
  {
    std::vector<TxtRecord> result;
    uint16_t count = TXTRecordGetCount(length, src);

    result.reserve(count);

    for (uint16_t index = 0; index < count; index++)
    {
      TxtRecord record;
      uint8_t valueLength = 0;
      const void *valueBuffer = nullptr;

      record.key.resize(256);

      auto error = TXTRecordGetItemAtIndex(length, src, index,
          record.key.size(), record.key.data(), &valueLength, &valueBuffer);

      if (error != kDNSServiceErr_NoError)
      {
#ifdef __cpp_exception
        throw std::logic_error("Should not happen.");
#else
        continue;
#endif
      }

      record.key.resize(strlen(record.key.c_str()));

      if (valueBuffer)
      {
        std::string tmp{
            reinterpret_cast<const char *>(valueBuffer), size_t{valueLength}};
        record.value.emplace(std::move(tmp));
      }

      result.emplace_back(std::move(record));
    }

    return result;
  }

  DnsSdResolve::DnsSdResolve(std::shared_ptr<DnsSdBrowser> browser,
      size_t interfaceIndex, const std::string &name, const std::string &type,
      const std::string &domain)
      : DnsSdRef(browser->getPlatform(),
            startResolve(interfaceIndex, name, type, domain, this)),
        browser_(browser), interfaceIndex_(interfaceIndex), name_(name),
        type_(type), domain_(domain)
  {
    std::cerr << describe() << std::endl;
  }

  DnsSdResolve::~DnsSdResolve() { std::cerr << "~" << describe() << std::endl; }

  std::string DnsSdResolve::describe() const
  {
    std::string result = "DnsSdResolve(";
    result += "if ";
    result += std::to_string(interfaceIndex_);
    result += ", ";
    result += name_;
    result += ", ";
    result += type_;
    result += ", ";
    result += domain_;
    result += ")";
    return result;
  }

  std::shared_ptr<DnsSdBrowser> DnsSdResolve::getBrowser() const
  {
    auto browser = browser_.lock();
    if (!browser)
    {
      MDNSCPP_THROW(
          std::logic_error, "DnsSdResolve detached from parent DnsSdBrowser");
    }
    return browser;
  }

  const std::string &DnsSdResolve::getName() const { return name_; }

  const std::string &DnsSdResolve::getType() const { return type_; }

  const std::string &DnsSdResolve::getDomain() const { return domain_; }

  size_t DnsSdResolve::getInterface() const { return interfaceIndex_; }

  const std::vector<TxtRecord> DnsSdResolve::getTxtRecords() const
  {
    return txtRecords_;
  }

  DNSServiceRef DnsSdResolve::startResolve(size_t interfaceIndex,
      const std::string &name, const std::string &type,
      const std::string &domain, void *context)
  {
    DNSServiceRef sdRef;

    auto error = DNSServiceResolve(&sdRef, 0,
        static_cast<uint32_t>(interfaceIndex), name.c_str(), type.c_str(),
        domain.c_str(), resolveResultCallback, context);

    if (kDNSServiceErr_NoError != error)
      MDNSCPP_THROW(std::runtime_error, "Failed.");

    return sdRef;
  }

  void DnsSdResolve::onResult(DNSServiceFlags flags,
      uint32_t interfaceIndexIndex, DNSServiceErrorType errorCode,
      const char *fullname, const char *hosttarget, uint16_t port,
      uint16_t txtLen, const unsigned char *txtRecord)
  {
    if (errorCode != kDNSServiceErr_NoError)
    {
      std::cerr << describe() << " failed with error " << errorCode
                << std::endl;
    }
    else
    {
      std::cerr << describe() << "Resolved " << fullname << " to " << hosttarget
                << " with " << txtLen << " bytes of txt records " << std::endl;

      txtRecords_ = parseTxtRecords(txtRecord, txtLen);

      getaddrinfo_ = std::make_shared<DnsSdGetAddrInfo>(
          shared_from_this(), interfaceIndexIndex, hosttarget, htons(port));

      close();
    }
  }

  void DnsSdResolve::resolveResultCallback(DNSServiceRef sdRef,
      DNSServiceFlags flags, uint32_t interfaceIndexIndex,
      DNSServiceErrorType errorCode, const char *fullname,
      const char *hosttarget, uint16_t port, uint16_t txtLen,
      const unsigned char *txtRecord, void *context)
  {
    reinterpret_cast<DnsSdResolve *>(context)->onResult(flags,
        interfaceIndexIndex, errorCode, fullname, hosttarget, port, txtLen,
        txtRecord);
  }
} // namespace mdnscpp
