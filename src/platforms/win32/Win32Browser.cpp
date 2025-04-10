#include "Win32Browser.h"

#include "Win32Platform.h"

#include "../../throw.h"

#include "../../debug.h"
#include <cwchar>
#include <stdexcept>
#include <string_view>

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
      MDNSCPP_INFO << dnsTypeToString(entry->wType) << " ttl: " << entry->dwTtl
                   << MDNSCPP_ENDL;
      switch (entry->wType)
      {
      case DNS_TYPE_PTR: {
        // FIXME: why does this select the const char * version??
        result.queryName =
            fromWideString((const wchar_t *)entry->Data.PTR.pNameHost);
        result.ttl = entry->dwTtl;
        MDNSCPP_INFO << "hostname: '" << result.queryName << "'"
                     << MDNSCPP_ENDL;
        break;
      }

      case DNS_TYPE_A: {
        result.addresses.emplace_back(&entry->Data.A.IpAddress);
        MDNSCPP_INFO
            << "ip4: '"
            << result.addresses[result.addresses.size() - 1].getDecimalString()
            << "'" << MDNSCPP_ENDL;
        break;
      }

      case DNS_TYPE_AAAA: {
        result.addresses.emplace_back(&entry->Data.AAAA.Ip6Address);
        MDNSCPP_INFO
            << "ip6: '"
            << result.addresses[result.addresses.size() - 1].getDecimalString()
            << "'" << MDNSCPP_ENDL;
        break;
      }
      case DNS_TYPE_SRV: {
        // FIXME: why does this select the const char * version??
        result.hostname =
            fromWideString((const wchar_t *)entry->Data.SRV.pNameTarget);
        result.port = entry->Data.SRV.wPort;
        MDNSCPP_INFO << "srv: '" << result.hostname << "' port: " << result.port
                     << MDNSCPP_ENDL;
        break;
      }
      }
    }

    return result;
  }

  void Win32Browser::DnsQueryCompletionRoutine(
      void *pQueryContext, DNS_QUERY_RESULT *queryResults)
  {
    MDNSCPP_INFO << "DnsQueryCompletionRoutine " << queryResults->QueryStatus
                 << MDNSCPP_ENDL;
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
        queue_(platform->getEventLoop()), platform_(platform),
        queryName_(getType() + "." + getProtocol() + "." + getDomain())
  {
    MDNSCPP_INFO << describe() << MDNSCPP_ENDL;
    DNS_SERVICE_BROWSE_REQUEST request;

    auto queryName = toWideString(queryName_);

    MDNSCPP_INFO << "queryName.size() == " << queryName.size() << MDNSCPP_ENDL;
    MDNSCPP_INFO << "browse for:" << fromWideString(queryName) << MDNSCPP_ENDL;

    request.Version = 2; /*DNS_QUERY_REQUEST_VERSION2;*/
    request.InterfaceIndex = static_cast<unsigned long>(interfaceIndex);
    request.QueryName = queryName.c_str();
    request.pQueryContext = this;
    request.pBrowseCallbackV2 = DnsQueryCompletionRoutine;

    auto status = DnsServiceBrowse(&request, &cancel_);

    if (status != DNS_REQUEST_PENDING)
    {
      MDNSCPP_ERROR << describe() << ": failed." << MDNSCPP_ENDL;
      MDNSCPP_THROW(std::runtime_error, "DnsServiceBrowse failed.");
    }
  }

  Win32Browser::~Win32Browser()
  {
    MDNSCPP_INFO << "~Win32Browser() start" << MDNSCPP_ENDL;
    auto status = DnsServiceBrowseCancel(&cancel_);
    if (status != ERROR_SUCCESS)
    {
      MDNSCPP_ERROR << "DnsServiceBrowseCancel() failed" << MDNSCPP_ENDL;
    }
    MDNSCPP_INFO << "~Win32Browser() done" << MDNSCPP_ENDL;
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

  std::string Win32Browser::stripQueryName(std::string fullname) const
  {
    MDNSCPP_INFO << "stripQueryName(" << fullname
                 << ") queryName: " << queryName_ << MDNSCPP_ENDL;
    if (fullname.size() < queryName_.size() + 1)
    {
      return "";
    }

    size_t nameLength = fullname.size() - queryName_.size() - 1;

    std::string_view end{fullname};

    end.remove_prefix(nameLength + 1);

    if (end != queryName_)
    {
      MDNSCPP_INFO << "end: " << end << MDNSCPP_ENDL;
      return "";
    }

    fullname.resize(nameLength);

    return fullname;
  }

  void Win32Browser::onBrowseResult(Win32BrowseResult result)
  {
    if (!result.queryName.size())
      return;

    queue_.schedule([result = std::move(result), this]() {
      auto it = resolves_.find(result.queryName);

      if (true || result.ttl)
      {
        if (it == resolves_.end())
        {
          it = resolves_
                   .insert({result.queryName,
                       std::make_shared<Win32Resolve>(*this, result.queryName,
                           stripQueryName(result.queryName))})
                   .first;
        }
        it->second->refresh(result.ttl);
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
