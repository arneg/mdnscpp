cmake_minimum_required(VERSION 3.16)

# C++-only tests for self-contained pieces of mdnscpp. The tests compile the
# sources under test directly instead of linking libmdnscpp, so that they do
# not need libuv, avahi or a platform mDNS stack, and can be built on their
# own:
#
#   cmake -S tests -B build/tests
#   cmake --build build/tests
#   ctest --test-dir build/tests --output-on-failure
#
# (or from the main project with -DMDNSCPP_BUILD_TESTS=ON)

project(mdnscpp-tests LANGUAGES CXX)

enable_testing()

set(MDNSCPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_executable(mdnscpp-tests
  main.cpp
  TestHarness.cpp
  TestHarness.h
  sockAddrToStringTest.cpp
  "${MDNSCPP_SRC}/sockAddrToString.cpp"
  validationTest.cpp
  "${MDNSCPP_SRC}/validation.cpp"
  browseResultTest.cpp
  "${MDNSCPP_SRC}/BrowseResult.cpp"
  "${MDNSCPP_SRC}/TxtRecord.cpp"
  guardCallbackTest.cpp
)

target_compile_features(mdnscpp-tests PRIVATE cxx_std_17)
target_include_directories(mdnscpp-tests PRIVATE
  "${MDNSCPP_SRC}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

if(WIN32)
  target_sources(mdnscpp-tests PRIVATE
    win32/WideStringTest.cpp
    "${MDNSCPP_SRC}/platforms/win32/toWideString.cpp"
    "${MDNSCPP_SRC}/platforms/win32/fromWideString.cpp"
  )
  target_compile_definitions(mdnscpp-tests PRIVATE
    LIBMDNS_PLATFORM_WIN32
    UNICODE
    _UNICODE
  )
  target_link_libraries(mdnscpp-tests PRIVATE Ws2_32)
endif()

add_test(NAME mdnscpp-tests COMMAND mdnscpp-tests)
