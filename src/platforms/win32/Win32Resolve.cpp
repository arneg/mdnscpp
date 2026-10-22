#pragma once

#include <string_view>

#include "Win32Browser.h"
#include "Win32Platform.h"
#include "Win32Resolve.h"
#include "fromWideString.h"
#include "toWideString.h"

#include "../../debug.h"
#include "../../guardCallback.h"

static constexpr uint64_t pendingRetryInterval = 5 * 1000;

// Time in ms after the ttl expires for which we keep the result
// around.
static constexpr uint64_t pendingStaleInterval = 5 * 1000;

static constexpr uint16_t minimumRefreshInterval = 30 * 1000;

// Time in ms after the ttl expires for which we keep trying to
// resolve the service.
static constexpr uint16_t pendingRemovalInterval = 10 * 1000;

namespace mdnscpp
{

  Win32Resolve::Win32Resolve(
      Win32Browser &browser, std::string queryName, std::string name)
      : browser_(browser), queryName_(queryName),
        queue_(browser_.getPlatform()->getEventLoop()), name_(name)
  {
    MDNSCPP_INFO << describe() << MDNSCPP_ENDL;
  }

  std::string Win32Resolve::describe() const
  {
    std::string result = "Win32Resolve(";
    result += browser_.describe();
    result += ", ";
    result += queryName_;
    result += ")";
    return result;
  }

  void Win32Resolve::resetResultTime() { resultTime_ = now(); }

  void Win32Resolve::resetResolveTime() { resolveTime_ = now(); }

  uint64_t Win32Resolve::now() const
  {
    return browser_.getPlatform()->getEventLoop().now();
  }

  uint64_t Win32Resolve::resolveAge() const { return now() - resolveTime_; }

  uint64_t Win32Resolve::resultAge() const { return now() - resultTime_; }

  bool Win32Resolve::hasResult() const { return ip4Result_ || ip6Result_; }

  bool Win32Resolve::isFresh() const
  {

    if (!ip4Result_ && !ip6Result_)
      return false;

    return now() < ttlTime_ && resultAge() < minimumRefreshInterval;
  }

  bool Win32Resolve::isStale() const
  {
    return ttlTime_ + pendingStaleInterval < now();
  }

  bool Win32Resolve::isGone() const
  {
    return ttlTime_ + pendingRemovalInterval < now();
  }

  void Win32Resolve::resetTtl(uint32_t ttl) { ttlTime_ = now() + ttl * 1000; }

  void Win32Resolve::refresh(uint32_t ttl)
  {
    MDNSCPP_INFO << describe() << ".refresh(" << ttl << ")"
                 << " age: " << (hasResult() ? resultAge() : 0) << MDNSCPP_ENDL;

    if (ttl)
      resetTtl(ttl);

    if (isFresh())
    {
      MDNSCPP_INFO << describe() << " Result is still fresh." << MDNSCPP_ENDL;
      return;
    }

    if (state_ == State::PENDING)
    {
      if (resolveAge() < pendingRetryInterval)
      {
        MDNSCPP_INFO << describe() << " Resolve is still pending for less than "
                     << pendingRetryInterval << ". Retry later."
                     << MDNSCPP_ENDL;
        return;
      }

      cancel();

      if (isStale())
      {
        updateResult(ip4Result_, nullptr);
        updateResult(ip6Result_, nullptr);
      }
    }

    state_ = State::PENDING;
    resetResolveTime();
    MDNSCPP_INFO << describe() << " Calling DnsServiceResolve." << MDNSCPP_ENDL;

    DNS_SERVICE_RESOLVE_REQUEST request;

    // queryName_ originates from the network, so this can fail.
    auto wideQueryName = toWideString(queryName_);
    if (!wideQueryName)
    {
      MDNSCPP_ERROR << describe()
                    << " Query name has invalid encoding. Not resolving."
                    << MDNSCPP_ENDL;
      state_ = State::FAILED;
      return;
    }

    request.Version = DNS_QUERY_REQUEST_VERSION1;
    request.InterfaceIndex =
        static_cast<unsigned long>(browser_.getInterface());
    request.QueryName = wideQueryName->data();
    request.pQueryContext = this;
    request.pResolveCompletionCallback = DnsServiceResolveComplete;

    auto status = DnsServiceResolve(&request, &cancel_);

    if (status != DNS_REQUEST_PENDING)
    {
      MDNSCPP_INFO << describe() << " DnsServiceResolve failed."
                   << MDNSCPP_ENDL;
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
      MDNSCPP_INFO << describe() << " Result unchanged. Skipping update."
                   << MDNSCPP_ENDL;
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
      ipv6.emplace(pInstance->ip6Address, pInstance->wPort);

    // Everything in pInstance comes from the network. Strings with invalid
    // encoding are dropped: if the hostname or instance name is invalid the
    // whole result is ignored, an invalid TXT entry is skipped.
    std::vector<TxtRecord> txtRecords;
    auto optHostname = fromWideString(pInstance->pszHostName);
    size_t interfaceIndex = pInstance->dwInterfaceIndex;
    auto optInstanceName = fromWideString(pInstance->pszInstanceName);
    uint16_t port = pInstance->wPort;

    if (!optHostname || !optInstanceName)
    {
      MDNSCPP_ERROR << describe()
                    << " Ignoring resolve result with invalid hostname or "
                       "instance name encoding."
                    << MDNSCPP_ENDL;
      return;
    }

    std::string hostname = std::move(*optHostname);
    std::string instanceName = std::move(*optInstanceName);

    if (instanceName != queryName_)
    {
      MDNSCPP_INFO << describe() << " instanceName " << instanceName
                   << " does not match the queryName " << queryName_
                   << MDNSCPP_ENDL;
      return;
    }

    // Do not trust the property count if the arrays are missing.
    const DWORD propertyCount =
        pInstance->keys ? pInstance->dwPropertyCount : 0;

    txtRecords.reserve(propertyCount);
    for (DWORD i = 0; i < propertyCount; i++)
    {
      TxtRecord record;

      auto key = fromWideString(pInstance->keys[i]);
      // A missing value (null pointer) yields an empty string, which means
      // "key without value". An invalid encoding drops the whole entry.
      auto value =
          fromWideString(pInstance->values ? pInstance->values[i] : nullptr);

      if (!key || !value)
      {
        MDNSCPP_ERROR << describe()
                      << " Ignoring TXT entry with invalid encoding."
                      << MDNSCPP_ENDL;
        continue;
      }

      if (key->empty())
        continue;

      record.key = std::move(*key);

      if (value->size())
        record.value = std::move(*value);

      txtRecords.push_back(std::move(record));
    }

    queue_.schedule([ipv4, ipv6, txtRecords = std::move(txtRecords),
                        hostname = std::move(hostname), port, interfaceIndex,
                        this]() {
      if (state_ != State::PENDING)
      {
        MDNSCPP_INFO << describe()
                     << "Got resolve result in unexpected state. ignoring."
                     << MDNSCPP_ENDL;
        return;
      }

      resetResultTime();
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
      guardCallback("Win32Resolve::DnsServiceResolveComplete",
          [&] { self->onResolveResult(pInstance); });
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

    MDNSCPP_INFO << "~" << describe() << MDNSCPP_ENDL;

    updateResult(ip4Result_, nullptr);
    updateResult(ip6Result_, nullptr);
  }
} // namespace mdnscpp
