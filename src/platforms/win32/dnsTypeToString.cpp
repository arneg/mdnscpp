#include "dnsTypeToString.h"

#include <Windows.h>
// windows.h comes first.
#include <WinDNS.h>

namespace mdnscpp
{
  const char *dnsTypeToString(int type)
  {

#define MDNSCPP_X(NAME)                                                        \
  if (type == NAME)                                                            \
    return #NAME;
#include "dnsTypeList.h"
#undef MDNSCPP_X
    return "unknown";
  }
} // namespace mdnscpp