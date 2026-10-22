#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace mdnscpp
{
  /**
   * Describes why the arguments for a browse are invalid.
   */
  struct BrowseArgumentError
  {
    enum class Argument
    {
      Type,
      Protocol,
      Domain,
      InterfaceIndex,
    };

    Argument argument;
    std::string message;
  };

  /**
   * Checks the arguments of Platform::createBrowser().
   *
   * - type: "_" followed by 1 to 15 letters, digits or hyphens (RFC 6763
   *   service name), e.g. "_http". Subtypes are not supported.
   * - protocol: exactly "_tcp" or "_udp".
   * - domain: empty (the default domain is used) or a DNS name in
   *   presentation format without escapes: valid UTF-8, no control
   *   characters, no backslashes, labels of 1 to 63 bytes, at most 253 bytes
   *   in total (an optional trailing dot is accepted).
   * - interfaceIndex: 0 (all interfaces) up to 2147483647.
   *
   * Returns std::nullopt if all arguments are valid. Never throws, so it can
   * be used regardless of whether exceptions are enabled.
   *
   * Platform::createBrowser() performs the same checks and throws
   * std::out_of_range (interfaceIndex) or std::invalid_argument (all other
   * arguments) if they fail.
   */
  std::optional<BrowseArgumentError> validateBrowseArguments(
      const std::string &type, const std::string &protocol,
      const std::string &domain, size_t interfaceIndex);
} // namespace mdnscpp
