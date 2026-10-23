#pragma once

// Standalone code uses PollLoop. LibuvLoop is for hosts that already own a
// uv_loop_t; include LibuvLoop.h directly in that case. Finding libuv at
// configure time must not change DefaultLoop.
#ifdef LIBMDNS_LOOP_POLL
#  include "PollLoop.h"
#elif defined(LIBMDNS_LOOP_LIBUV)
#  include "LibuvLoop.h"
#else
#  error "No event loop supported."
#endif

namespace mdnscpp
{
#ifdef LIBMDNS_LOOP_POLL
  using DefaultLoop = PollLoop;
#elif defined(LIBMDNS_LOOP_LIBUV)
  using DefaultLoop = LibuvLoop;
#endif
} // namespace mdnscpp
