#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <mdnscpp/Browser.h>

namespace mdnscpp
{
  class Win32Platform;

  class Win32Browser : public Browser,
                       public std::enable_shared_from_this<Win32Browser>
  {
  public:
    Win32Browser(std::shared_ptr<Win32Platform> platform,
        const std::string &type, const std::string &protocol,
        ResultsChangedCallback onResultsChanged, const std::string &domain,
        size_t interfaceIndex, IPProtocol ipProtocol);

    ~Win32Browser();

    std::string describe() const;

  protected:
    std::shared_ptr<Browser> getSharedFromThis() override;

  private:
  };
} // namespace mdnscpp
