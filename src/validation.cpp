#include <mdnscpp/validation.h>

#include <cstdint>

namespace mdnscpp
{
  namespace
  {
    using Argument = BrowseArgumentError::Argument;

    // Limits from RFC 1035 / RFC 6335 / RFC 6763.
    constexpr size_t maxServiceNameLength = 15;
    constexpr size_t maxLabelLength = 63;
    // Length of a name in presentation format, without the trailing dot.
    constexpr size_t maxNameLength = 253;
    constexpr size_t maxInterfaceIndex = 0x7FFFFFFF;

    BrowseArgumentError makeError(Argument argument, const char *message)
    {
      return BrowseArgumentError{argument, message};
    }

    bool isServiceNameChar(char c)
    {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || c == '-';
    }

    // Strict: rejects overlong encodings, surrogates and values above
    // U+10FFFF.
    bool isValidUtf8(const std::string &s)
    {
      size_t i = 0;
      const size_t n = s.size();

      while (i < n)
      {
        const unsigned char c = static_cast<unsigned char>(s[i]);

        if (c < 0x80)
        {
          i++;
          continue;
        }

        size_t length;
        uint32_t codePoint;

        if (c >= 0xC2 && c <= 0xDF)
        {
          length = 2;
          codePoint = c & 0x1F;
        }
        else if (c >= 0xE0 && c <= 0xEF)
        {
          length = 3;
          codePoint = c & 0x0F;
        }
        else if (c >= 0xF0 && c <= 0xF4)
        {
          length = 4;
          codePoint = c & 0x07;
        }
        else
        {
          return false;
        }

        if (i + length > n)
          return false;

        for (size_t k = 1; k < length; k++)
        {
          const unsigned char continuation =
              static_cast<unsigned char>(s[i + k]);
          if ((continuation & 0xC0) != 0x80)
            return false;
          codePoint = (codePoint << 6) | (continuation & 0x3F);
        }

        if (length == 3 &&
            (codePoint < 0x800 || (codePoint >= 0xD800 && codePoint <= 0xDFFF)))
          return false;
        if (length == 4 && (codePoint < 0x10000 || codePoint > 0x10FFFF))
          return false;

        i += length;
      }

      return true;
    }

    std::optional<BrowseArgumentError> validateType(const std::string &type)
    {
      if (type.size() < 2 || type.size() > 1 + maxServiceNameLength ||
          type[0] != '_')
        return makeError(Argument::Type,
            "type must be '_' followed by 1 to 15 letters, digits or "
            "hyphens, e.g. '_http'.");

      for (size_t i = 1; i < type.size(); i++)
      {
        if (!isServiceNameChar(type[i]))
          return makeError(Argument::Type,
              "type must be '_' followed by 1 to 15 letters, digits or "
              "hyphens, e.g. '_http'.");
      }

      return std::nullopt;
    }

    std::optional<BrowseArgumentError> validateProtocol(
        const std::string &protocol)
    {
      if (protocol != "_tcp" && protocol != "_udp")
        return makeError(
            Argument::Protocol, "protocol must be '_tcp' or '_udp'.");

      return std::nullopt;
    }

    std::optional<BrowseArgumentError> validateDomain(
        const std::string &domain, size_t *nameLength)
    {
      *nameLength = 0;

      // The default domain is used.
      if (domain.empty())
        return std::nullopt;

      std::string name = domain;

      // One trailing dot (fully qualified name) is accepted.
      if (name.back() == '.')
        name.pop_back();

      if (name.empty())
        return makeError(Argument::Domain, "domain must not be empty.");

      if (!isValidUtf8(name))
        return makeError(Argument::Domain, "domain is not valid UTF-8.");

      for (char c : name)
      {
        const unsigned char u = static_cast<unsigned char>(c);
        if (u < 0x20 || u == 0x7F)
          return makeError(
              Argument::Domain, "domain must not contain control characters.");
        if (c == '\\')
          return makeError(
              Argument::Domain, "domain must not contain backslashes.");
      }

      if (name.size() > maxNameLength)
        return makeError(
            Argument::Domain, "domain is too long (at most 253 bytes).");

      size_t labelLength = 0;
      for (char c : name)
      {
        if (c == '.')
        {
          if (labelLength == 0)
            return makeError(
                Argument::Domain, "domain must not contain empty labels.");
          labelLength = 0;
        }
        else if (++labelLength > maxLabelLength)
        {
          return makeError(Argument::Domain,
              "domain contains a label longer than 63 bytes.");
        }
      }

      if (labelLength == 0)
        return makeError(
            Argument::Domain, "domain must not contain empty labels.");

      *nameLength = name.size();
      return std::nullopt;
    }
  } // namespace

  std::optional<BrowseArgumentError> validateBrowseArguments(
      const std::string &type, const std::string &protocol,
      const std::string &domain, size_t interfaceIndex)
  {
    if (auto error = validateType(type))
      return error;

    if (auto error = validateProtocol(protocol))
      return error;

    size_t domainLength;
    if (auto error = validateDomain(domain, &domainLength))
      return error;

    // The name that is actually queried: <type>.<protocol>.<domain>
    if (type.size() + 1 + protocol.size() + 1 + domainLength > maxNameLength)
      return makeError(
          Argument::Domain, "type, protocol and domain together are too long.");

    if (interfaceIndex > maxInterfaceIndex)
      return makeError(Argument::InterfaceIndex,
          "interfaceIndex must be between 0 and 2147483647.");

    return std::nullopt;
  }
} // namespace mdnscpp
