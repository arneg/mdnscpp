# Fetch a pinned libuv. Included only when MDNSCPP_FETCH_LIBUV is ON and
# find_package did not find a system copy.

include(FetchContent)

message(STATUS "Fetching libuv v1.53.0")

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "Build shared libuv" FORCE)

FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv.git
  GIT_TAG v1.53.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libuv)

set(libuv_FOUND TRUE)
