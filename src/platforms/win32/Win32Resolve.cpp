#pragma once

#include <string_view>

#include "Win32Browser.h"
#include "Win32Platform.h"
#include "Win32Resolve.h"
#include "fromWideString.h"
#include "toWideString.h"

#include "../../debug.h"

static constexpr uint64_t pendingRetryInterval = 5 * 1000;

namespace mdnscpp
{

  Win32Resolve::Win32Resolve(
      Win32Browser &browser, std::string queryName, std::string name)
      : browser_(browser), queryName_(queryName),
        queue_(browser_.getPlatform()->getEventLoop()), name_(name)
  {
    MDNSCPP_INFO << "Win32Resolve(" << browser_.describe() << ", " << queryName
                 << ")" << MDNSCPP_ENDL;
  }

  void Win32Resolve::resetUpdateTime()
  {
    updateTime_ = browser_.getPlatform()->getEventLoop().now();
  }

  uint64_t Win32Resolve::age() const
  {
    return browser_.getPlatform()->getEventLoop().now() - updateTime_;
  }

  bool Win32Resolve::isFresh() const
  {
    if (state_ != State::DONE)
    {
      return false;
    }

    auto ageInSeconds = age() / 1000;

    return ageInSeconds < ttl_ / 2;
  }

  void Win32Resolve::refresh(uint32_t ttl)
  {
    MDNSCPP_INFO << "Win32Resolve(" << queryName_ << ").refresh(" << ttl << ")"
                 << "age: " << age() << MDNSCPP_ENDL;

    if (state_ == State::PENDING)
    {
      if (age() < pendingRetryInterval)
      {
        MDNSCPP_INFO << "Resolve is still pending for less than "
                     << pendingRetryInterval << ". Retry later."
                     << MDNSCPP_ENDL;
        SleepEx(100, TRUE);
        return;
      }

      cancel();
    }

    ttl_ = (std::min)(ttl_, ttl);

    if (isFresh())
    {
      MDNSCPP_INFO << "Result is still fresh." << MDNSCPP_ENDL;
      return;
    }

    ttl_ = ttl;
    state_ = State::PENDING;
    resetUpdateTime();
    MDNSCPP_INFO << "Calling DnsServiceResolve." << MDNSCPP_ENDL;

    DNS_SERVICE_RESOLVE_REQUEST request;

    std::wstring wideQueryName = toWideString(queryName_);

    request.Version = DNS_QUERY_REQUEST_VERSION1;
    request.InterfaceIndex =
        static_cast<unsigned long>(browser_.getInterface());
    request.QueryName = wideQueryName.data();
    request.pQueryContext = this;
    request.pResolveCompletionCallback = DnsServiceResolveComplete;

    auto status = DnsServiceResolve(&request, &cancel_);

    if (status != DNS_REQUEST_PENDING)
    {
      MDNSCPP_INFO << "DnsServiceResolve failed." << MDNSCPP_ENDL;
      state_ = State::FAILED;
    }
  }

  void Win32Resolve::updateResult(
      std::shared_ptr<BrowseResult> &slot, std::shared_ptr<BrowseResult> result)
  {
    if (slot == result)
      return;
    if (slot && result && *slot == *result)
    {
      MDNSCPP_INFO << "Result unchanged. Skipping update." << MDNSCPP_ENDL;
      return;
    }

    if (slot)
    {
      browser_.removeResult(slot);
    }

    std::swap(slot, result);

    if (slot)
    {
      browser_.insertResult(slot);
    }
  }

  void Win32Resolve::onResolveResult(PDNS_SERVICE_INSTANCE pInstance)
  {
    std::optional<IPAddress> ipv4, ipv6;

    if (pInstance->ip4Address)
      ipv4.emplace(pInstance->ip4Address, pInstance->wPort);

    if (pInstance->ip6Address)
      ipv4.emplace(pInstance->ip6Address, pInstance->wPort);

    std::vector<TxtRecord> txtRecords;
    std::string hostname = fromWideString(pInstance->pszHostName);
    size_t interfaceIndex = pInstance->dwInterfaceIndex;
    std::string instanceName = fromWideString(pInstance->pszInstanceName);
    uint16_t port = pInstance->wPort;

    if (instanceName != queryName_)
    {
      MDNSCPP_INFO << "instanceName " << instanceName
                   << " does not match the queryName " << queryName_
                   << MDNSCPP_ENDL;
      return;
    }

    for (DWORD i = 0; i < pInstance->dwPropertyCount; i++)
    {
      TxtRecord record;

      record.key = fromWideString(pInstance->keys[i]);

      if (!record.key.size())
        continue;

      std::string tmp = fromWideString(pInstance->values[i]);

      if (tmp.size())
        record.value = std::move(tmp);

      txtRecords.push_back(std::move(record));
    }

    queue_.schedule([ipv4, ipv6, txtRecords, hostname, port, interfaceIndex,
                        this]() {
      if (state_ != State::PENDING)
      {
        MDNSCPP_INFO << "Got resolve result in unexpected state. ignoring."
                     << MDNSCPP_ENDL;
        return;
      }

      resetUpdateTime();
      state_ = State::DONE;

      auto makeResult = [&](const IPAddress &ip) {
        return std::make_shared<BrowseResult>(txtRecords, browser_.getType(),
            browser_.getProtocol(), name_, browser_.getDomain(), hostname,
            ip.getDecimalString(), interfaceIndex, ip.getType(), port);
      };

      updateResult(ip4Result_, ipv4 ? makeResult(*ipv4) : nullptr);
      updateResult(ip6Result_, ipv6 ? makeResult(*ipv6) : nullptr);
    });
  }

  void Win32Resolve::DnsServiceResolveComplete(
      DWORD Status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE pInstance)
  {
    if (pInstance)
    {
      Win32Resolve *self = reinterpret_cast<Win32Resolve *>(pQueryContext);
      self->onResolveResult(pInstance);
      DnsServiceFreeInstance(pInstance);
    }
  }

  void Win32Resolve::cancel()
  {
    if (state_ == State::PENDING)
    {
      DnsServiceResolveCancel(&cancel_);
      state_ = State::INIT;
    }
  }

  Win32Resolve::~Win32Resolve()
  {
    cancel();

    MDNSCPP_INFO << "~Win32Resolve(" << browser_.describe() << ", "
                 << queryName_ << ")" << MDNSCPP_ENDL;

    if (ip4Result_)
      browser_.removeResult(ip4Result_);
    if (ip6Result_)
      browser_.removeResult(ip6Result_);
  }
} // namespace mdnscpp