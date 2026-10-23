cmake_minimum_required(VERSION 3.16)

project(libmdnscpp-example LANGUAGES CXX)

# Standalone: cmake -S example -B build/example
#
# Windows has no PollLoop. Turn on the library's fetch option so a missing
# system libuv is downloaded. -DMDNSCPP_FETCH_LIBUV=OFF keeps find_package
# only.
if(WIN32)
  set(MDNSCPP_FETCH_LIBUV ON CACHE BOOL
    "Fetch a pinned libuv if find_package does not find one")
endif()

if(NOT TARGET libmdnscpp)
  add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/mdnscpp")
endif()

add_executable(libmdnscpp-example)

target_sources(libmdnscpp-example PRIVATE
  main.cpp
)

target_link_libraries(libmdnscpp-example PRIVATE libmdnscpp)
