#pragma once

#ifdef LIBMDNS_LOOP_LIBUV
#  include "LibuvLoop.h"
#elif LIBMDNS_LOOP_POLL
#  include "PollLoop.h"
#else
#  error "No event loop supported."
#endif

namespace mdnscpp
{
#ifdef LIBMDNS_LOOP_LIBUV
  using DefaultLoop = LibuvLoop;
#elif LIBMDNS_LOOP_POLL
  using DefaultLoop = PollLoop;
#endif
} // namespace mdnscpp