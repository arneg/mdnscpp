#include <mdnscpp/TxtRecord.h>

namespace mdnscpp
{
  bool TxtRecord::operator==(const TxtRecord &other) const
  {
    return key == other.key && value == other.value;
  }

  bool TxtRecord::operator!=(const TxtRecord &other) const
  {
    return !(*this == other);
  }
} // namespace mdnscpp
