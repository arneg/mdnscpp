#include "Win32Browser.h"

#include "Win32Platform.h"

#include <cwchar>
#include <iostream>
#include <stdexcept>

#include "dnsTypeToString.h"
#include "fromWideString.h"
#include "toWideString.h"

namespace mdnscpp
{

  static Win32BrowseResult dnsRecordToBrowseResult(DNS_RECORD *records)
  {
    Win32BrowseResult result;

    for (DNS_RECORD *entry = records; entry; entry = entry->pNext)
    {
      std::cerr << dnsTypeToString(entry->wType) << " ttl: " << entry->dwTtl
                << std::endl;
      switch (entry->wType)
      {
      case DNS_TYPE_PTR: {
        // FIXME: why does this select the const char * version??
        result.queryName =
            fromWideString((const wchar_t *)entry->Data.PTR.pNameHost);
        result.ttl = entry->dwTtl;
        std::cerr << "hostname: '" << result.queryName << "'" << std::endl;
        break;
      }

      case DNS_TYPE_A: {
        result.addresses.emplace_back(&entry->Data.A.IpAddress);
        std::cerr
            << "ip4: '"
            << result.addresses[result.addresses.size() - 1].getDecimalString()
            << "'" << std::endl;
        break;
      }

      case DNS_TYPE_AAAA: {
        result.addresses.emplace_back(&entry->Data.AAAA.Ip6Address);
        std::cerr
            << "ip6: '"
            << result.addresses[result.addresses.size() - 1].getDecimalString()
            << "'" << std::endl;
        break;
      }
      case DNS_TYPE_SRV: {
        // FIXME: why does this select the const char * version??
        result.hostname =
            fromWideString((const wchar_t *)entry->Data.SRV.pNameTarget);
        result.port = entry->Data.SRV.wPort;
        std::cerr << "srv: '" << result.hostname << "' port: " << result.port
                  << std::endl;
        break;
      }
      }
    }

    return result;
  }

  void Win32Browser::DnsQueryCompletionRoutine(
      void *pQueryContext, DNS_QUERY_RESULT *queryResults)
  {
    DNS_RECORD *records = queryResults->pQueryRecords;

    Win32Browser *browser = reinterpret_cast<Win32Browser *>(pQueryContext);
    Win32BrowseResult result = dnsRecordToBrowseResult(records);

    DnsRecordListFree(records, DnsFreeRecordList);

    browser->onBrowseResult(std::move(result));
  }

  static const std::string defaultDomain = "local";

  Win32Browser::Win32Browser(std::shared_ptr<Win32Platform> platform,
      const std::string &type, const std::string &protocol,
      ResultsChangedCallback onResultsChanged, const std::string &domain,
      size_t interfaceIndex, IPProtocol ipProtocol)
      : Browser(type, protocol, onResultsChanged,
            domain.size() ? domain : defaultDomain, interfaceIndex, ipProtocol),
        queue_(platform->getEventLoop()), platform_(platform)
  {
    std::cerr << describe() << std::endl;
    DNS_SERVICE_BROWSE_REQUEST request;

    auto queryName =
        toWideString(getType() + "." + getProtocol() + "." + getDomain());

    request.Version = 2; /*DNS_QUERY_REQUEST_VERSION2;*/
    request.InterfaceIndex = static_cast<unsigned long>(interfaceIndex);
    request.QueryName = queryName.c_str();
    request.pQueryContext = this;
    request.pBrowseCallbackV2 = DnsQueryCompletionRoutine;

    auto status = DnsServiceBrowse(&request, &cancel_);

    if (status != DNS_REQUEST_PENDING)
    {
      std::cerr << describe() << ": failed." << std::endl;
      throw std::runtime_error("DnsServiceBrowse failed.");
    }
  }

  Win32Browser::~Win32Browser()
  {
    std::cerr << "~Win32Browser()" << std::endl;
    auto status = DnsServiceBrowseCancel(&cancel_);
    if (status != ERROR_SUCCESS)
    {
      std::cerr << "DnsServiceBrowseCancel() failed" << std::endl;
    }
  }

  std::string Win32Browser::describe() const
  {
    std::string result = "Win32Browse(";
    result += type_;
    result += ", ";
    result += protocol_;
    result += ", ";
    result += domain_;
    result += ", ";
    result += std::to_string(interfaceIndex_);
    result += ")";
    return result;
  }

  void Win32Browser::onBrowseResult(Win32BrowseResult result)
  {
    if (!result.queryName.size())
      return;

    queue_.schedule([result = std::move(result), this]() {
      auto shared_this =
          std::static_pointer_cast<Win32Browser>(getSharedFromThis());
      auto it = resolves_.find(result.queryName);

      if (result.ttl)
      {
        if (it == resolves_.end())
        {
          resolves_.insert({result.queryName,
              std::make_shared<Win32Resolve>(shared_this, result.queryName)});
        }
      }
      else
      {
        if (it != resolves_.end())
        {
          resolves_.erase(it);
        }
      }
    });
  }

  std::shared_ptr<Browser> Win32Browser::getSharedFromThis()
  {
    return shared_from_this();
  }

  std::shared_ptr<Win32Platform> Win32Browser::getPlatform() const
  {
    return platform_;
  }

} // namespace mdnscpp
